#include "Global.h"
#include "UrfCompactionManager.h"

#include "MemStore.h"
#include "DiskStore.h"
#include "CompactionManager.h"
#include "KVTMapInputStream.h"
#include "KVTDiskFile.h"
#include "KVTDiskFileInputStream.h"
#include "KVTDiskFileOutputStream.h"
#include "KVTPriorityInputStream.h"

#define MERGE_ONLINE 0

/*========================================================================
 *                           UrfCompactionManager
 *========================================================================*/
UrfCompactionManager::UrfCompactionManager(MemStore *memstore, DiskStore *diskstore)
: CompactionManager(memstore, diskstore)
{

}

/*========================================================================
 *                          ~UrfCompactionManager
 *========================================================================*/
UrfCompactionManager::~UrfCompactionManager()
{

}

/*========================================================================
 *                            flush_memstore
 *========================================================================*/
void UrfCompactionManager::flush_bytes(void)
{
    // -------------------------------- NOTE ----------------------------------
    // every new file created by Compaction Manager should be inserted
    // at the *back* of the vector, so the last file in vector is the most
    // recent one, and the first is the oldest one.
    // the same should be done accordingly for their respective input streams
    // -------------------------------- NOTE ----------------------------------

    sanity_check();

    sanity_check();
}

/*=======================================================================*
 *                              sanity_check
 *=======================================================================*/
void UrfCompactionManager::sanity_check()
{
    return_if_dbglvl_lt_2();

    assert(m_diskstore->m_disk_files.size() == m_diskstore->m_disk_istreams.size());
}
