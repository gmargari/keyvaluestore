#include "Global.h"
#include "RangemergeCompactionManager.h"

#include "MemStore.h"
#include "DiskStore.h"
#include "MapInputStream.h"
#include "DiskFile.h"
#include "DiskFileInputStream.h"
#include "Range.h"

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <climits>

#include <algorithm>

using std::vector;
using std::min;

#define SPLIT_PERC 0.6 // move to Global.h?

/*============================================================================
 *                        RangemergeCompactionManager
 *============================================================================*/
RangemergeCompactionManager::RangemergeCompactionManager(MemStore *memstore, DiskStore *diskstore)
    : CompactionManager(memstore, diskstore),
      m_blocksize(DEFAULT_RNGMERGE_BLOCKSIZE), m_flushmem(DEFAULT_RNGMERGE_FLUSHMEMSIZE)
{

}

/*============================================================================
 *                       ~RangemergeCompactionManager
 *============================================================================*/
RangemergeCompactionManager::~RangemergeCompactionManager()
{

}

/*============================================================================
 *                               set_blocksize
 *============================================================================*/
void RangemergeCompactionManager::set_blocksize(uint64_t blocksize)
{
    if (blocksize) {
        m_blocksize = blocksize;
    } else {
        // if blocksize == 0 block splitting is disabled and Rangemerge behaves
        // identically to Immediate Merge
        m_blocksize = ULONG_MAX; // practically, infinite block size
    }
}

/*============================================================================
 *                               get_blocksize
 *============================================================================*/
uint64_t RangemergeCompactionManager::get_blocksize(void)
{
    return m_blocksize;
}

/*============================================================================
 *                               set_flushmem
 *============================================================================*/
void RangemergeCompactionManager::set_flushmem(uint64_t flushmem)
{
    m_flushmem = min(flushmem, m_memstore->get_maxsize());
}

/*============================================================================
 *                               get_flushmem
 *============================================================================*/
uint64_t RangemergeCompactionManager::get_flushmem(void)
{
    return m_flushmem;
}

/*============================================================================
 *                               flush_bytes
 *============================================================================*/
void RangemergeCompactionManager::flush_bytes(void)
{
    DiskFileInputStream *disk_stream;
    vector<DiskFile *> new_disk_files;
    vector<DiskFile *> &r_disk_files = m_diskstore->m_disk_files;
    vector<DiskFileInputStream *> &r_disk_istreams = m_diskstore->m_disk_istreams;
    vector<InputStream *> istreams_to_merge;
    vector<Range> ranges;
    Range rng;
    uint64_t bytes_flushed, memstore_size, memstore_maxsize;
    int i, cur_rng, newfiles, idx;
#if DBGLVL > 0
    uint64_t dbg_memsize, dbg_memsize_serial;
#endif

    assert(sanity_check());

    //--------------------------------------------------------------------------
    // create vector of ranges //TODO: can we keep this rangetable permanently and not recreate it every time?! we only need to update first, last & memsize
    //--------------------------------------------------------------------------
    create_ranges(ranges);

    //--------------------------------------------------------------------------
    // sort ranges by memory size
    //--------------------------------------------------------------------------
    sort(ranges.begin(), ranges.end(), Range::cmp_by_memsize);

    //--------------------------------------------------------------------------
    // flush 'm_flushmem' bytes
    //--------------------------------------------------------------------------
    cur_rng = 0;
    bytes_flushed = 0;
    memstore_size = m_memstore->get_size();
    memstore_maxsize = m_memstore->get_maxsize();
    do {
        assert(cur_rng < (int)ranges.size());
        rng = ranges[cur_rng];

        // get istream with all memory tuples that belong to current range
        m_memstore->m_inputstream->set_key_range(rng.m_first, rng.m_last, true, false);
        istreams_to_merge.clear();
        istreams_to_merge.push_back(m_memstore->m_inputstream);

        // get istream for file that stores all tuples that belong to range
        if (rng.m_idx != NO_DISK_FILE) {
            disk_stream = r_disk_istreams[rng.m_idx];
            disk_stream->set_key_range(rng.m_first, rng.m_last, true, false);
            istreams_to_merge.push_back(disk_stream);
        }

        // check if block will overflow. in case of overflow, range will be
        // split to a number of subranges, each of which will be stored on
        // a separate block.
        if (rng.m_memsize_serialized + rng.m_disksize > m_blocksize) {
            newfiles = merge_streams(istreams_to_merge, new_disk_files, SPLIT_PERC * m_blocksize);
            // assert(newfiles > 1); // this may not hold true (i.e. if all memory keys exist on disk and no split occurs)
        } else {
            // merge memory and disk istreams, creating a new file on disk.
            // set 'm_blocksize' as max file size, which will cause only
            // one file to be created (we have ensured aboce block will not
            // overflow)
            newfiles = merge_streams(istreams_to_merge, new_disk_files, m_blocksize);
            assert(newfiles == 1);
        }

        assert((dbg_memsize = m_memstore->get_size()) || 1);
        assert((dbg_memsize_serial = m_memstore->get_size_when_serialized()) || 1);

        // remove from memstore tuples of current range
        m_memstore->clear(rng.m_first, rng.m_last, true, false);

        assert(dbg_memsize - rng.m_memsize == m_memstore->get_size());
        assert(dbg_memsize_serial - rng.m_memsize_serialized == m_memstore->get_size_when_serialized());

        bytes_flushed += rng.m_memsize;
        cur_rng++;

    // flush ranges until memory size drops below 'memstore_maxsize - m_flushmem'
    } while (memstore_size - bytes_flushed > memstore_maxsize - m_flushmem);

    //--------------------------------------------------------------------------
    // delete old files from disk and remove them from diskstore
    //--------------------------------------------------------------------------
    // we flushed from memory to disk the first 'cur_rng' ranges. in order to
    // avoid problems with deletion below, sort ranges by 'm_idx' field and
    // delete files from last to first
    sort(ranges.begin(), ranges.begin() + cur_rng, Range::cmp_by_file_index);
    for (i = cur_rng - 1; i >= 0; i--) {
        idx = ranges[i].m_idx;
        if (idx != NO_DISK_FILE) {
            r_disk_files[idx]->delete_from_disk();
            delete r_disk_files[idx];
            delete r_disk_istreams[idx];
            r_disk_files.erase(r_disk_files.begin() + idx);
            r_disk_istreams.erase(r_disk_istreams.begin() + idx);
        }
    }

    //--------------------------------------------------------------------------
    // add new files to diskstore
    //--------------------------------------------------------------------------
    // doesn't matter where we insert them in vector: in Rangemerge each key is
    // stored in exaclty one file on disk, so there is no need to search
    // all files from most recent to oldest -we'll search in exaclty one file.
    for (i = 0; i < (int)new_disk_files.size(); i++) {
        r_disk_files.push_back(new_disk_files[i]);
        disk_stream = new DiskFileInputStream(m_diskstore->m_disk_files.back(), MERGE_BUFSIZE);
        r_disk_istreams.push_back(disk_stream);
    }

    assert(sanity_check());
}

/*============================================================================
 *                               create_ranges
 *============================================================================*/
void RangemergeCompactionManager::create_ranges(vector<Range>& ranges)
{
    vector<DiskFile *> &r_disk_files = m_diskstore->m_disk_files;
    Range rng;
    int i;
    pair<uint64_t, uint64_t> sizes;

    // if there are files on disk
    if (r_disk_files.size()) {

        // create ranges so that _each_ memory tuple belongs to _exactly one_ range:
        // - based on disk files, set for each range only the field 'first'
        // - sort ranges lexicographically by field 'first'
        // - range[0].first = NULL;
        // - range[N-1].last = NULL;
        // - for (i = 0; i < N-1; i++)
        //      range[i].last = range[i+1].first;
        // when using ranges, 'first' should be inclusive and 'last' exclusive
        for (i = 0; i < (int)r_disk_files.size(); i++) {
            r_disk_files[i]->get_first_last_term(&rng.m_first, &rng.m_last);
            rng.m_disksize = r_disk_files[i]->get_size();
            rng.m_idx = i;
            ranges.push_back(rng);
        }

        sort(ranges.begin(), ranges.end(), Range::cmp_by_term);

        ranges[0].m_first = NULL;
        ranges[ranges.size() - 1].m_last = NULL;

        for (i = 0; i < (int)ranges.size() - 1; i++) {
            ranges[i].m_last = ranges[i+1].m_first;
        }

        for (i = 0; i < (int)ranges.size(); i++) {
            // since we want to know exactly the size of tuples when written to
            // disk, in order to know when a block will overflow, we use
            // get_size_when_serialized() and not get_size().
            sizes = m_memstore->get_sizes(ranges[i].m_first, ranges[i].m_last, true, false);
            ranges[i].m_memsize = sizes.first;
            ranges[i].m_memsize_serialized = sizes.second;
        }
    }
    // else, if this is the first time we flush bytes to disk (no disk file)
    else {
        rng.m_first = NULL; // to get all memory tuples
        rng.m_last = NULL;
        sizes = m_memstore->get_sizes();
        rng.m_memsize = sizes.first;
        rng.m_memsize_serialized = sizes.second;
        rng.m_disksize = 0;
        rng.m_idx = NO_DISK_FILE;
        ranges.push_back(rng);
    }
}

/*============================================================================
 *                                sanity_check
 *============================================================================*/
int RangemergeCompactionManager::sanity_check()
{
    vector<Range> ranges;

    assert(m_diskstore->m_disk_files.size() == m_diskstore->m_disk_istreams.size());
    for (unsigned int i = 0; i < m_diskstore->m_disk_files.size(); i++) {
        assert(m_diskstore->m_disk_files[i]->get_size() <= m_blocksize);
    }

    // create vector of ranges
    create_ranges(ranges);

    // assert ranges are non interleaving, i.e. each key is stored in at most
    // one disk file, and that each memory tuple belongs to exaclty one range
    assert(ranges[0].m_first == NULL);
    assert(ranges[ranges.size() - 1].m_last == NULL);
    for (unsigned int i = 0; i < ranges.size(); i++) {
        assert(i == 0 || strcmp(ranges[i-1].m_last, ranges[i].m_first) == 0);
    }

    return 1;
}