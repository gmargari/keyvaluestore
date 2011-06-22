#include "Global.h"
#include "UrfCompactionManager.h"

#include "MemStore.h"
#include "DiskStore.h"
#include "CompactionManager.h"
#include "KVTMapInputStream.h"
#include "KVTDiskFile.h"
#include "KVTDiskFileInputStream.h"
#include "KVTDiskFileOutputStream.h"

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <sys/types.h>

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
 *                             set_blocksize
 *========================================================================*/
void UrfCompactionManager::set_blocksize(uint64_t blocksize)
{
    m_blocksize = blocksize;
}

/*========================================================================
 *                             get_blocksize
 *========================================================================*/
uint64_t UrfCompactionManager::get_blocksize(void)
{
    return m_blocksize;
}

/*========================================================================
 *                             set_flushmem
 *========================================================================*/
void UrfCompactionManager::set_flushmem(uint64_t flushmem)
{
    if (flushmem > m_memstore->get_maxsize()) {
        printf("Error: flushmem (%Ld) cannot be greater than max memory (%Ld)\n", flushmem, m_memstore->get_maxsize());
        exit(EXIT_FAILURE);
    }

    m_flushmem = flushmem;
}

/*========================================================================
 *                             get_flushmem
 *========================================================================*/
uint64_t UrfCompactionManager::get_flushmem(void)
{
    return m_flushmem;
}

/*========================================================================
 *                            flush_memstore
 *========================================================================*/
void UrfCompactionManager::flush_bytes(void)
{
    assert(sanity_check());

    assert(sanity_check());
}

/*=======================================================================*
 *                              sanity_check
 *=======================================================================*/
int UrfCompactionManager::sanity_check()
{
    assert(m_diskstore->m_disk_files.size() == m_diskstore->m_disk_istreams.size());
    return 1;
}
