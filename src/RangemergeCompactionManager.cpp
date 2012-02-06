// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#include "./Global.h"
#include "./RangemergeCompactionManager.h"

#include <assert.h>
#include <limits.h>
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
      m_blocksize(DEFAULT_RNG_BLOCKSIZE) {
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
 *                               flush_bytes
 *============================================================================*/
void RangemergeCompactionManager::flush_bytes() {
    MapInputStream *map_istream;
    vector<DiskFile *> new_disk_files;
    vector<InputStream *> istreams;
    Range rng;
    int newfiles;
#if DBGLVL > 0
    uint64_t dbg_memsize, dbg_memsize_serial;
#endif

    assert(sanity_check());

    //--------------------------------------------------------------------------
    // select a range to flush
    //--------------------------------------------------------------------------
    rng = get_best_range();

    //--------------------------------------------------------------------------
    // merge range's disk tuples with range's memory tuples
    //--------------------------------------------------------------------------
    // get istream for all memory tuples that belong to range
    map_istream = m_memstore->new_map_inputstream(rng.m_first);
    istreams.push_back(map_istream);

    // get istream for file that stores all tuples that belong to range
    if (rng.m_file_num != NO_DISK_FILE) {
        DiskFile *dfile = m_diskstore->get_diskfile(rng.m_file_num);
        istreams.push_back(new DiskFileInputStream(dfile));
    }

    // if block may overflow, split merged block into a number of blocks
    // of max file size 'SPLIT_PERC * m_blocksize'
    if (rng.m_memsize_serialized + rng.m_disksize > m_blocksize) {
        newfiles = Streams::merge_streams(istreams, &new_disk_files,
                                          SPLIT_PERC * m_blocksize);
        // assert(newfiles > 1);  // can be false (e.g. all mem keys exist on
                                  // disk -> values are overwritten -> no split)
    } else {
        newfiles = Streams::merge_streams(istreams, &new_disk_files,
                                          m_blocksize);
        assert(newfiles == 1);
    }

    // delete all istreams
    for (int i = 0; i < (int)istreams.size(); i++) {
        delete istreams[i];
    }

    assert((dbg_memsize = m_memstore->get_size()) || 1);
    assert((dbg_memsize_serial = m_memstore->get_size_when_serialized()) || 1);

    //--------------------------------------------------------------------------
    // update memstore
    //--------------------------------------------------------------------------
    // remove from memstore tuples of range
    m_memstore->clear_map(rng.m_first);

    // add new map(s) to memstore if new range(s) was created (range split)
    for (int i = 0; i < newfiles - 1; i++) {
        Slice ft, lt;
        new_disk_files[newfiles - i - 1]->get_first_last_term(&ft, &lt);
        m_memstore->add_map(ft);
    }

    assert(dbg_memsize - rng.m_memsize == m_memstore->get_size());
    assert(dbg_memsize_serial - rng.m_memsize_serialized
             == m_memstore->get_size_when_serialized());

    //--------------------------------------------------------------------------
    // update diskstore
    //--------------------------------------------------------------------------
    m_diskstore->write_lock();
    // delete old file from disk and remove from diskstore
    if (rng.m_file_num != NO_DISK_FILE) {
        m_diskstore->get_diskfile(rng.m_file_num)->delete_from_disk();
        m_diskstore->rm_diskfile(rng.m_file_num);
    }

    // add new file(s) to diskstore (doesn't matter where)
    for (int i = 0; i < newfiles; i++) {
        m_diskstore->add_diskfile(new_disk_files[i], 0);
    }
    m_diskstore->write_unlock();

    assert(sanity_check());
}

/*============================================================================
 *                               get_best_range
 *============================================================================*/
Range RangemergeCompactionManager::get_best_range() {
    vector<Range *> rngs;
    Range rng;

    // get range with max memory size
    create_ranges(&rngs);
    sort(rngs.begin(), rngs.end(), Range::cmp_by_memsize);
    rng = *(rngs[0]);
    delete_ranges(rngs);

    return rng;
}

/*============================================================================
 *                               create_ranges
 *============================================================================*/
void RangemergeCompactionManager::create_ranges(vector<Range *> *rngs) {
    int num_disk_files = m_diskstore->get_num_disk_files();
    Slice last;
    Map *map;

    rngs->clear();
    if (num_disk_files) {
        for (int i = 0; i < num_disk_files; i++) {
            DiskFile *dfile = m_diskstore->get_diskfile(i);
            Range *rng = new Range();
            dfile->get_first_last_term(&rng->m_first, &last);
            rng->m_disksize = dfile->get_size();
            rng->m_file_num = i;
            rngs->push_back(rng);
        }

        sort(rngs->begin(), rngs->end(), Range::cmp_by_term);

        rngs->at(0)->m_first = Slice("", 0);
        for (int i = 0; i < (int)rngs->size(); i++) {
            // m_memsize_serialized: we want to know the exact size of tuples
            // written to disk, in order to know when a block will overflow
            map = m_memstore->get_map(rngs->at(i)->m_first);
            rngs->at(i)->m_memsize = map->get_size();
            rngs->at(i)->m_memsize_serialized = map->get_size_when_serialized();
        }
    } else {  // if this is the first time we flush bytes to disk (no disk file)
        Range *rng = new Range();
        rng->m_first = Slice("", 0);  // to get all terms
        map = m_memstore->get_map(rng->m_first);
        rng->m_memsize = map->get_size();
        rng->m_memsize_serialized = map->get_size_when_serialized();
        rng->m_disksize = 0;
        rng->m_file_num = NO_DISK_FILE;
        rngs->push_back(rng);
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

    for (int i = 0; i < m_diskstore->get_num_disk_files(); i++) {
        assert(m_diskstore->get_diskfile(i)->get_size() <= m_blocksize);
    }

    create_ranges(&ranges);
    assert(m_memstore->get_num_maps() == (int)ranges.size());
    for (int i = 0; i < (int)ranges.size(); i++) {
        assert(m_memstore->get_map(i)
                 == m_memstore->get_map(ranges[i]->m_first));
    }
    assert(strcmp(ranges[0]->m_first.data(), "") == 0);
    delete_ranges(ranges);

    return 1;
}
