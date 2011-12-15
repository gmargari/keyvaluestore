// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_RANGEMERGECOMPACTIONMANAGER_H_
#define SRC_RANGEMERGECOMPACTIONMANAGER_H_

#include <stdint.h>
#include <vector>

#include "./Global.h"
#include "./CompactionManager.h"

using std::vector;

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

  private:
    /**
     * when loading an existing index from disk, create in memstore one map
     * for each range (add to memstore one map per range/disk file)
     */
    void add_maps_to_memstore();

    /**
     * based on disk files and memstore, create the vector of ranges. ranges
     * are sorted in lexicographical order
     */
    void create_ranges(vector<Range *>& ranges);
    void delete_ranges(vector<Range *> ranges);

    /**
     * save/load compaction manager state to/from disk
     */
    bool save_state_to_disk();
    bool load_state_from_disk();

    int sanity_check();

    uint64_t m_blocksize;
    uint64_t m_flushmem;
};

#endif  // SRC_RANGEMERGECOMPACTIONMANAGER_H_
