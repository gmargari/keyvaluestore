#ifndef NULL_COMPACTIONMANAGER_H
#define NULL_COMPACTIONMANAGER_H

#include "CompactionManager.h"
#include "pthread.h"

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

    // inherited from CompactionManager (see CompactionManager.h for description)
    void flush_bytes();
};

/*============================================================================
 *                               flush_memstore
 *============================================================================*/
void NomergeCompactionManager::flush_bytes() {
    DiskFile *memstore_file;

    memstore_file = memstore_flush_to_diskfile();
    memstore_clear();

    pthread_rwlock_wrlock(&m_diskstore->m_rwlock);
    // insert first in diskstore as it contains the most recent <k,v> pairs
    m_diskstore->m_disk_files.insert(m_diskstore->m_disk_files.begin(), memstore_file);
    pthread_rwlock_unlock(&m_diskstore->m_rwlock);
}

#endif
