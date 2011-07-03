#include "Global.h"
#include "NullCompactionManager.h"

#include "MemStore.h"
#include "DiskStore.h"
#include "DiskFile.h"
#include "DiskFileInputStream.h"

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
    DiskFile *memstore_file;
    DiskFileInputStream *memstore_file_istream;

    assert(sanity_check());

    memstore_file = memstore_flush_to_diskfile();
    memstore_clear();

    // insert first, in diskstore files vector & input streams vector, as it
    // contains the most recent <k,v> pairs
    m_diskstore->m_disk_files.insert(m_diskstore->m_disk_files.begin(), memstore_file);
    memstore_file_istream = new DiskFileInputStream(m_diskstore->m_disk_files.back(), MERGE_BUFSIZE);
    m_diskstore->m_disk_istreams.insert(m_diskstore->m_disk_istreams.begin(), memstore_file_istream);

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
