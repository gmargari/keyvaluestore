#include "Global.h"
#include "NullCompactionManager.h"

#include "MemStore.h"
#include "DiskStore.h"

#include <cassert>

/*============================================================================
 *                             NullCompactionManager
 *============================================================================*/
NullCompactionManager::NullCompactionManager(MemStore *memstore, DiskStore *diskstore)
: CompactionManager(memstore, diskstore)
{

}

/*============================================================================
 *                            ~NullCompactionManager
 *============================================================================*/
NullCompactionManager::~NullCompactionManager()
{

}

/*============================================================================
 *                               flush_memstore
 *============================================================================*/
void NullCompactionManager::flush_bytes(void)
{
    assert(sanity_check());

    memstore_flush_to_new_diskfile();
    memstore_clear();

    assert(sanity_check());
}

/*============================================================================
 *                                sanity_check
 *============================================================================*/
int NullCompactionManager::sanity_check()
{
    assert(m_diskstore->m_disk_files.size() == m_diskstore->m_disk_istreams.size());

    return 1;
}
