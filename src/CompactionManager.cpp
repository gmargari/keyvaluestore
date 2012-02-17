// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#include "./Global.h"
#include "./CompactionManager.h"

#include <assert.h>

#include "./MemStore.h"
#include "./MapInputStream.h"
#include "./DiskFile.h"
#include "./DiskFileOutputStream.h"
#include "./Streams.h"

/*============================================================================
 *                              CompactionManager
 *============================================================================*/
CompactionManager::CompactionManager(MemStore *memstore, DiskStore *diskstore)
    : m_memstore(memstore), m_diskstore(diskstore) {
}

/*============================================================================
 *                              ~CompactionManager
 *============================================================================*/
CompactionManager::~CompactionManager() {
}

/*============================================================================
 *                        memstore_flush_to_diskfile
 *============================================================================*/
DiskFile *CompactionManager::memstore_flush_to_diskfile() {
    DiskFile *disk_file;
    DiskFileOutputStream *disk_ostream;
    MapInputStream *map_istream;

    // write memstore to a new file on disk
    disk_file = new DiskFile();
    disk_file->open_new_unique();
    disk_ostream = new DiskFileOutputStream(disk_file, MERGEBUF_SIZE);
    map_istream = m_memstore->new_map_inputstream();
    // no need to use copy_stream_unique_keys() since map keys are unique
    Streams::copy_stream(map_istream, disk_ostream);
    delete map_istream;
    delete disk_ostream;

    assert(m_memstore->get_size_when_serialized() == disk_file->get_size());
    m_memstore->clear_map();

    return disk_file;
}

/*============================================================================
 *                               request_flush
 *============================================================================*/
void CompactionManager::request_flush() {
    do_flush();
}
