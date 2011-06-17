#include "Global.h"
#include "LogCompactionManager.h"

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
 *                           LogCompactionManager
 *========================================================================*/
LogCompactionManager::LogCompactionManager(MemStore *memstore, DiskStore *diskstore)
: CompactionManager(memstore, diskstore)
{

}

/*========================================================================
 *                          ~LogCompactionManager
 *========================================================================*/
LogCompactionManager::~LogCompactionManager()
{

}

/*========================================================================
 *                            flush_memstore
 *========================================================================*/
void LogCompactionManager::flush_bytes(void)
{
    // -------------------------------- NOTE ----------------------------------
    // every new file created by Compaction Manager should be inserted
    // at the *back* of the vector, so the last file in vector is the most
    // recent one, and the first is the oldest one.
    // the same should be done accordingly for their respective input streams
    // -------------------------------- NOTE ----------------------------------

    assert(sanity_check());

    assert(sanity_check());
}

/*=======================================================================*
 *                              sanity_check
 *=======================================================================*/
int LogCompactionManager::sanity_check()
{
    assert(m_diskstore->m_disk_files.size() == m_diskstore->m_disk_istreams.size());

    return 1;
}
