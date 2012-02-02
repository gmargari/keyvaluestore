// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_NOMERGECOMPACTIONMANAGER_H_
#define SRC_NOMERGECOMPACTIONMANAGER_H_

#include "./CompactionManager.h"

class NomergeCompactionManager: public CompactionManager {
  public:
    /**
     * constructor
     */
    NomergeCompactionManager(MemStore *memstore, DiskStore *diskstore)
        : CompactionManager(memstore, diskstore) { }

    /**
     * destructor
     */
    ~NomergeCompactionManager() { }

    // inherited from CompactionManager (see CompactionManager.h for info)
    void flush_bytes();
};

/*============================================================================
 *                                flush_bytes
 *============================================================================*/
void NomergeCompactionManager::flush_bytes() {
    DiskFile *dfile;

    dfile = memstore_flush_to_diskfile();
    memstore_clear();

    m_diskstore->write_lock();
    m_diskstore->add_diskfile(dfile, 0);  // insert 1st, has the most recent KVs
    m_diskstore->write_unlock();
}

#endif  // SRC_NOMERGECOMPACTIONMANAGER_H_
