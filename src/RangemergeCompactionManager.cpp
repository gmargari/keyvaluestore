// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#include "./RangemergeCompactionManager.h"

#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <algorithm>
#include <utility>
#include <vector>

#include "./MemStore.h"
#include "./DiskStore.h"
#include "./MapInputStream.h"
#include "./DiskFile.h"
#include "./DiskFileInputStream.h"
#include "./Range.h"
#include "./Streams.h"

using std::vector;
using std::min;

#define SPLIT_PERC 0.6  // move to Global.h?

/*============================================================================
 *                        RangemergeCompactionManager
 *============================================================================*/
RangemergeCompactionManager::RangemergeCompactionManager(MemStore *memstore,
                                                         DiskStore *diskstore)
    : CompactionManager(memstore, diskstore),
      m_blocksize(DEFAULT_RNG_BLOCKSIZE), m_flushmem(DEFAULT_RNG_FLUSHMEMSIZE) {
    load_state_from_disk();
    add_maps_to_memstore();
}

/*============================================================================
 *                       ~RangemergeCompactionManager
 *============================================================================*/
RangemergeCompactionManager::~RangemergeCompactionManager() {
    save_state_to_disk();
}

/*============================================================================
 *                               set_blocksize
 *============================================================================*/
void RangemergeCompactionManager::set_blocksize(uint64_t blocksize) {
    if (blocksize) {
        m_blocksize = blocksize;
    } else {
        // if blocksize == 0, block splitting is disabled and Rangemerge behaves
        // identically to Immediate Merge
        m_blocksize = ULONG_MAX;  // practically, infinite block size
    }
}

/*============================================================================
 *                               get_blocksize
 *============================================================================*/
uint64_t RangemergeCompactionManager::get_blocksize() {
    return m_blocksize;
}

/*============================================================================
 *                               set_flushmem
 *============================================================================*/
void RangemergeCompactionManager::set_flushmem(uint64_t flushmem) {
    m_flushmem = flushmem;
}

/*============================================================================
 *                               get_flushmem
 *============================================================================*/
uint64_t RangemergeCompactionManager::get_flushmem() {
    return m_flushmem;
}

/*============================================================================
 *                               flush_bytes
 *============================================================================*/
void RangemergeCompactionManager::flush_bytes() {
    MapInputStream *map_istream;
    DiskFileInputStream *disk_stream;
    vector<DiskFile *> new_disk_files;
    vector<DiskFile *> &disk_files = m_diskstore->m_disk_files;
    vector<InputStream *> istreams;
    vector<Range *> ranges;
    Range *rng;
    uint64_t bytes_flushed, memstore_size, memstore_maxsize;
    int i, cur_rng, newfiles;
#if DBGLVL > 0
    uint64_t dbg_memsize, dbg_memsize_serial;
#endif

    assert(sanity_check());

    //--------------------------------------------------------------------------
    // create vector of ranges
    //--------------------------------------------------------------------------
    create_ranges(&ranges);

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
        map_istream = m_memstore->new_map_inputstream(rng->m_first);
        istreams.clear();
        istreams.push_back(map_istream);

        // get istream for file that stores all tuples that belong to range
        if (rng->m_block_num != NO_DISK_BLOCK) {
            disk_stream = new DiskFileInputStream(disk_files[rng->m_block_num]);
            istreams.push_back(disk_stream);
        }

        // check if block will overflow. in case of overflow, range will be
        // split to a number of subranges, each of which will be stored on
        // a separate block.
        if (rng->m_memsize_serialized + rng->m_disksize > m_blocksize) {
            newfiles = Streams::merge_streams(istreams, &new_disk_files,
                                              SPLIT_PERC * m_blocksize);
            // assert(newfiles > 1);  // this may be false (i.e. if all mem keys
                                      // exist on disk and no split occurs)
        } else {
            // merge memory and disk istreams, creating a new file on disk.
            // set 'm_blocksize' as max file size, which will cause only
            // one file to be created (we have ensured aboce block will not
            // overflow)
            newfiles = Streams::merge_streams(istreams, &new_disk_files,
                                              m_blocksize);
            assert(newfiles == 1);
        }

        // delete all istreams
        for (i = 0; i < (int)istreams.size(); i++) {
            delete istreams[i];
        }

        assert((dbg_memsize = m_memstore->get_size()) || true);
        assert((dbg_memsize_serial = m_memstore->get_size_when_serialized())
                 || true);

        // remove from memstore tuples of current range
        m_memstore->clear_map(rng->m_first);

        // add new map(s) to memstore if new range(s) was created (range split)
        for (int j = 0; j < newfiles - 1; j++) {
            int new_file_index;
            Slice ft, lt;

            new_file_index = new_disk_files.size() - j - 1;
            new_disk_files.at(new_file_index)->get_first_last_term(&ft, &lt);
            m_memstore->add_map(ft);
        }

        assert(dbg_memsize - rng->m_memsize == m_memstore->get_size());
        assert(dbg_memsize_serial - rng->m_memsize_serialized
                 == m_memstore->get_size_when_serialized());

        bytes_flushed += rng->m_memsize;
        cur_rng++;

    // flush ranges until memory size drops below 'memstore_maxsize - flushmem'
    } while (memstore_size - bytes_flushed > memstore_maxsize - m_flushmem);

    //--------------------------------------------------------------------------
    // delete old files from disk and remove them from diskstore
    //--------------------------------------------------------------------------
    // we flushed to disk the first 'cur_rng' ranges. in order to avoid problems
    // with file deletion, sort first 'cur_rng' ranges by 'block_num' and delete
    // files from last to first (note: we store each block in a separate disk
    // file. 'm_block_num' is the index of file in diskfiles vector)
    sort(ranges.begin(), ranges.begin() + cur_rng, Range::cmp_by_block_num);
    m_diskstore->write_lock();
    for (i = cur_rng - 1; i >= 0; i--) {
        int blocknum = ranges[i]->m_block_num;
        if (blocknum != NO_DISK_BLOCK) {
            disk_files[blocknum]->delete_from_disk();
            delete disk_files[blocknum];
            disk_files.erase(disk_files.begin() + blocknum);
        }
    }
    delete_ranges(ranges);

    //--------------------------------------------------------------------------
    // add new files to diskstore
    //--------------------------------------------------------------------------
    // doesn't matter where we insert them in vector: in Rangemerge each key is
    // stored in exaclty one file on disk, so there is no need to search
    // all files from most recent to oldest -we'll search in exaclty one file.
    for (i = 0; i < (int)new_disk_files.size(); i++) {
        disk_files.push_back(new_disk_files[i]);
    }
    m_diskstore->write_unlock();

    assert(sanity_check());
}

/*============================================================================
 *                               create_ranges
 *============================================================================*/
void RangemergeCompactionManager::create_ranges(vector<Range *> *ranges) {
    vector<DiskFile *> disk_files = m_diskstore->m_disk_files;
    Range *rng;
    Slice last;
    pair<uint64_t, uint64_t> sizes;

    if (disk_files.size()) {  // if there are files on disk
        for (unsigned int i = 0; i < disk_files.size(); i++) {
            rng = new Range();
            disk_files[i]->get_first_last_term(&rng->m_first, &last);
            rng->m_disksize = disk_files[i]->get_size();
            rng->m_block_num = i;
            ranges->push_back(rng);
        }

        sort(ranges->begin(), ranges->end(), Range::cmp_by_term);

        ranges->at(0)->m_first = Slice("", 0);

        for (unsigned int i = 0; i < ranges->size(); i++) {
            // since we want to know exactly the size of tuples when written to
            // disk, in order to know when a block will overflow, we use
            // get_size_when_serialized() and not get_size().
            sizes = m_memstore->get_map(ranges->at(i)->m_first)->get_sizes();
            ranges->at(i)->m_memsize = sizes.first;
            ranges->at(i)->m_memsize_serialized = sizes.second;
        }
    } else {  // if this is the first time we flush bytes to disk (no disk file)
        rng = new Range();
        rng->m_first = Slice("", 0);  // to get all terms
        sizes = m_memstore->get_map(rng->m_first)->get_sizes();
        rng->m_memsize = sizes.first;
        rng->m_memsize_serialized = sizes.second;
        rng->m_disksize = 0;
        rng->m_block_num = NO_DISK_BLOCK;
        ranges->push_back(rng);
    }
}

/*============================================================================
 *                               delete_ranges
 *============================================================================*/
void RangemergeCompactionManager::delete_ranges(vector<Range *> ranges) {
    for (int i = 0; i < (int)ranges.size(); i++) {
        delete ranges[i];
    }
}

/*============================================================================
 *                              save_state_to_disk
 *============================================================================*/
bool RangemergeCompactionManager::save_state_to_disk() {
    char fname[100];
    FILE *fp;

    sprintf(fname, "%s%s", ROOT_DIR, CMMANAGER_FILENAME);
    if ((fp = fopen(fname, "w")) == NULL) {
        printf("Error: fopen('%s')\n", fname);
        perror("");
        exit(EXIT_FAILURE);
    }

    fprintf(fp, "cmmanager: %s\n", "rangemerge");
    fprintf(fp, "blocksize: %Ld\n", m_blocksize);
    fprintf(fp, "flushmem: %Ld\n", m_flushmem);
    fclose(fp);

    return true;
}

/*============================================================================
 *                            load_state_from_disk
 *============================================================================*/
bool RangemergeCompactionManager::load_state_from_disk() {
    char fname[100], cmmanager[100];
    FILE *fp;

    // open existing diskstore, if any
    sprintf(fname, "%s%s", ROOT_DIR, CMMANAGER_FILENAME);
    if ((fp = fopen(fname, "r")) != NULL) {
        fscanf(fp, "cmmanager: %s\n", cmmanager);
        if (strcmp(cmmanager, "rangemerge") != 0) {
            printf("Error: expected 'rangemerge' cmanager in %s, found '%s'\n",
                   fname, cmmanager);
            exit(EXIT_FAILURE);
        }
        fscanf(fp, "blocksize: %Ld\n", &m_blocksize);
        fscanf(fp, "flushmem: %Ld\n", &m_flushmem);
        fclose(fp);

        return true;
    }

    return false;
}

/*============================================================================
 *                            add_maps_to_memstore
 *============================================================================*/
void RangemergeCompactionManager::add_maps_to_memstore() {
    vector<Range *> ranges;

    create_ranges(&ranges);
    // i = 1: MemStore constructor has already inserted map for key ""
    for (int i = 1; i < (int)ranges.size(); i++) {
        m_memstore->add_map(ranges[i]->m_first);
    }
    delete_ranges(ranges);

    sanity_check();
}

/*============================================================================
 *                                sanity_check
 *============================================================================*/
int RangemergeCompactionManager::sanity_check() {
    vector<Range *> ranges;

    for (unsigned int i = 0; i < m_diskstore->m_disk_files.size(); i++) {
        assert(m_diskstore->m_disk_files[i]->get_size() <= m_blocksize);
    }
    create_ranges(&ranges);
    assert(m_memstore->get_num_maps() == (int)ranges.size());
    for (unsigned int i = 0; i < ranges.size(); i++) {
        assert(m_memstore->get_map(i)
                 == m_memstore->get_map(ranges[i]->m_first));
    }

    // assert ranges are non interleaving, i.e. each key is stored in at most
    // one disk file, and that each memory tuple belongs to exaclty one range
    assert(strcmp(ranges[0]->m_first.data(), "") == 0);
    delete_ranges(ranges);

    return 1;
}
