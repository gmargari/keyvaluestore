// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_COMPACTIONMANAGER_H_
#define SRC_COMPACTIONMANAGER_H_

#include "./Global.h"
#include "./leveldb/slice.h"

class MemStore;
class DiskStore;
class DiskFile;

class CompactionManager {
  public:
    /**
     * constructor
     */
    CompactionManager(MemStore *memstore, DiskStore *diskstore);

    /**
     * destructor
     */
    virtual ~CompactionManager();

    /**
     * flush memstore to disk, creating a new disk file, and clear memstore.
     * return pointer to new disk file created
     */
    DiskFile *memstore_flush_to_diskfile();
    DiskFile *memstore_flush_to_diskfile(Slice key);

    /**
     * request a memory flush, to create some free space in memory
     */
    void request_flush();

    // Undefined methods (just remove Weffc++ warning)
    CompactionManager(const CompactionManager&);
    CompactionManager& operator=(const CompactionManager&);

  protected:
    /**
     * flush KVs from memory to disk, creating free space for new KVs
     */
    virtual void do_flush() = 0;

    MemStore    *m_memstore;
    DiskStore   *m_diskstore;
};

#endif  // SRC_COMPACTIONMANAGER_H_
