#include "Global.h"
#include "CompactionManager.h"

#include "MemStore.h"
#include "DiskStore.h"
#include "MapInputStream.h"
#include "DiskFile.h"
#include "DiskFileInputStream.h"
#include "DiskFileOutputStream.h"
#include "PriorityInputStream.h"
#include "Buffer.h"
#include "Statistics.h"
#include "Streams.h"

/*============================================================================
 *                              CompactionManager
 *============================================================================*/
CompactionManager::CompactionManager(MemStore *memstore, DiskStore *diskstore)
    : m_memstore(memstore), m_diskstore(diskstore)
{

}

/*============================================================================
 *                              ~CompactionManager
 *============================================================================*/
CompactionManager::~CompactionManager()
{

}

/*============================================================================
 *                        memstore_flush_to_diskfile
 *============================================================================*/
DiskFile *CompactionManager::memstore_flush_to_diskfile()
{
    DiskFile *disk_file;
    DiskFileOutputStream *disk_ostream;
    MapInputStream *map_istream;

    // write memstore to a new file on disk
    disk_file = new DiskFile();
    disk_file->open_new_unique();
    disk_ostream = new DiskFileOutputStream(disk_file, MERGE_BUFSIZE);
    map_istream = new MapInputStream(m_memstore->m_map);
    // no need to use copy_stream_unique_keys() since map keys are unique
    Streams::copy_stream(map_istream, disk_ostream);
    delete map_istream;
    delete disk_ostream;

    assert(m_memstore->get_size_when_serialized() == disk_file->get_size());

    return disk_file;
}

/*============================================================================
 *                              memstore_clear
 *============================================================================*/
void CompactionManager::memstore_clear()
{
    m_memstore->clear();
}
