// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_NOMERGECOMPACTIONMANAGER_H_
#define SRC_NOMERGECOMPACTIONMANAGER_H_

#include "./CompactionManager.h"
#include "./pthread.h"

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
 *                               flush_memstore
 *============================================================================*/
void NomergeCompactionManager::flush_bytes() {
    DiskFile *dfile;

    dfile = memstore_flush_to_diskfile();
    memstore_clear();

    m_diskstore->write_lock();
    // insert first in diskstore as it contains the most recent <k,v> pairs
    m_diskstore->m_disk_files.insert(m_diskstore->m_disk_files.begin(), dfile);
    m_diskstore->write_unlock();
}

#endif  // SRC_NOMERGECOMPACTIONMANAGER_H_
