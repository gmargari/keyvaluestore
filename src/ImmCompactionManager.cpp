#include "Global.h"
#include "ImmCompactionManager.h"

#include "MemStore.h"
#include "DiskStore.h"
#include "CompactionManager.h"
#include "KVTMapInputStream.h"
#include "KVTDiskFile.h"
#include "KVTDiskFileInputStream.h"
#include "KVTDiskFileOutputStream.h"
#include "KVTPriorityInputStream.h"

const bool ONLINE_MEMSTORE_MERGE = true;

/*========================================================================
 *                           ImmCompactionManager
 *========================================================================*/
ImmCompactionManager::ImmCompactionManager(MemStore *memstore, DiskStore *diskstore)
: CompactionManager(memstore, diskstore)
{

}

/*========================================================================
 *                          ~ImmCompactionManager
 *========================================================================*/
ImmCompactionManager::~ImmCompactionManager()
{

}

/*========================================================================
 *                            flush_memstore
 *========================================================================*/
void ImmCompactionManager::flush_bytes(void)
{
    KVTDiskFile *disk_file;
    KVTDiskFileOutputStream *disk_ostream;
    vector<KVTInputStream *> istreams;

    // -------------------------------- NOTE ----------------------------------
    // every new file created by Compaction Manager should be inserted
    // at the *front* of the vector, so the first file in vector is the most
    // recent one, and the last is the oldest one.
    // the same should be done accordingly for their respective input streams
    // -------------------------------- NOTE ----------------------------------

    assert(sanity_check());

    // if we perform offline merge, flush memstore to a new file on disk
    if (ONLINE_MEMSTORE_MERGE == false) {
        memstore_flush_to_new_diskfile();
        memstore_clear();

        // first time memstore is flushed, no disk file to merge with
        if (m_diskstore->m_disk_istreams.size() == 1) {
            assert(sanity_check());
            return;
        }
    }
    // else, add memstore stream first since it contains the most recent <k,v> pairs
    else {
        m_memstore->m_inputstream->reset();
        istreams.push_back(m_memstore->m_inputstream);
    }

    // add all disk input streams that will be merged, from most recent to oldest
    for (int i = 0; i < (int)m_diskstore->m_disk_istreams.size(); i++) {
        m_diskstore->m_disk_istreams[i]->reset();
        istreams.push_back(m_diskstore->m_disk_istreams[i]);
    }

    // merge input streams, writing output to a new file
    disk_file = new KVTDiskFile;
    disk_file->open_unique();
    disk_ostream = new KVTDiskFileOutputStream(disk_file);
    merge_streams(istreams, disk_ostream);
    delete disk_ostream;

    // clear memstore
    if (ONLINE_MEMSTORE_MERGE) {
        memstore_clear();
    }

    // delete all files merged as well as their input streams
    for (int i = 0; i < (int)m_diskstore->m_disk_files.size(); i++) {
        m_diskstore->m_disk_files[i]->delete_from_disk();
        delete m_diskstore->m_disk_files[i];
        delete m_diskstore->m_disk_istreams[i];
    }
    m_diskstore->m_disk_files.clear();
    m_diskstore->m_disk_istreams.clear();

    // add new file and its input stream to (currently empty) set of disk files
    m_diskstore->m_disk_files.push_back(disk_file);
    m_diskstore->m_disk_istreams.push_back(new KVTDiskFileInputStream(disk_file));

    assert(sanity_check());
}

/*=======================================================================*
 *                              sanity_check
 *=======================================================================*/
int ImmCompactionManager::sanity_check()
{
    assert(m_diskstore->m_disk_files.size() == m_diskstore->m_disk_istreams.size());
    assert(m_diskstore->m_disk_istreams.size() <= 1);

    return 1;
}
