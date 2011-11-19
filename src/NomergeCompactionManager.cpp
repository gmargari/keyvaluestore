#include "Global.h"
#include "NomergeCompactionManager.h"

#include "MemStore.h"
#include "DiskStore.h"
#include "DiskFile.h"
#include "DiskFileInputStream.h"

#include <cassert>
#include <pthread.h>

/*============================================================================
 *                             NomergeCompactionManager
 *============================================================================*/
NomergeCompactionManager::NomergeCompactionManager(MemStore *memstore, DiskStore *diskstore)
    : CompactionManager(memstore, diskstore)
{

}

/*============================================================================
 *                            ~NomergeCompactionManager
 *============================================================================*/
NomergeCompactionManager::~NomergeCompactionManager()
{

}

/*============================================================================
 *                               flush_memstore
 *============================================================================*/
void NomergeCompactionManager::flush_bytes()
{
    DiskFile *memstore_file;

    memstore_file = memstore_flush_to_diskfile();
    memstore_clear();

    pthread_rwlock_wrlock(&m_diskstore->m_rwlock);
    // insert first in diskstore as it contains the most recent <k,v> pairs
    m_diskstore->m_disk_files.insert(m_diskstore->m_disk_files.begin(), memstore_file);
    pthread_rwlock_unlock(&m_diskstore->m_rwlock);
}
