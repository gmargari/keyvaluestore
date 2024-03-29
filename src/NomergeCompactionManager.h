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

  private:
    // inherited from CompactionManager (see CompactionManager.h for info)
    void do_flush();
};

/*============================================================================
 *                                do_flush
 *============================================================================*/
void NomergeCompactionManager::do_flush() {
    DiskFile *disk_file = memstore_flush_to_diskfile();

    m_diskstore->write_lock();
    m_diskstore->add_diskfile(disk_file, 0);  // insert 1st, has most recent KVs
    m_diskstore->write_unlock();
}

#endif  // SRC_NOMERGECOMPACTIONMANAGER_H_
