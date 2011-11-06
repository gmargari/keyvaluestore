#ifndef RANGEMERGE_COMPACTIONMANAGER_H
#define RANGEMERGE_COMPACTIONMANAGER_H

#include "CompactionManager.h"

#include <stdint.h>

class MemStore;
class DiskStore;
class Range;

class RangemergeCompactionManager: public CompactionManager {

public:

    /**
     * constructor
     */
    RangemergeCompactionManager(MemStore *memstore, DiskStore *diskstore);

    /**
     * destructor
     */
    ~RangemergeCompactionManager();

    /**
     * set/get the size of disk block
     */
    void     set_blocksize(uint64_t blocksize);
    uint64_t get_blocksize();

    /**
     * set/get the size of memory flushed each time memory gets full
     */
    void     set_flushmem(uint64_t flushmem);
    uint64_t get_flushmem();

    // inherited from CompactionManager (see CompactionManager.h for description)
    void flush_bytes();

protected:

    /**
     * based on disk files and memstore, create the vector of ranges. ranges
     * are sorted in lexicographical order
     */
    void create_ranges(vector<Range>& ranges);

    /**
     * save/load compaction manager state to/from disk
     */
    bool save_state_to_disk();
    bool load_state_from_disk();

    int sanity_check();

    uint64_t m_blocksize;
    uint64_t m_flushmem;
};

#endif
