#include "Global.h"
#include "ImmCompactionManager.h"

#include "MemStore.h"
#include "DiskStore.h"
#include "KVTMapInputStream.h"
#include "KVTDiskFile.h"
#include "KVTDiskFileInputStream.h"

#include <cassert>

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
    vector<KVTInputStream *> istreams_to_merge;
    vector<KVTDiskFile *> &r_disk_files = m_diskstore->m_disk_files;
    vector<KVTDiskFileInputStream *> &r_disk_istreams = m_diskstore->m_disk_istreams;

    assert(sanity_check());

    // if we perform offline merge, flush memstore to a new file on disk
    if (get_memstore_merge_type() == CM_MERGE_OFFLINE) {
        memstore_flush_to_new_diskfile();
        memstore_clear();

        // first time memstore is flushed, no disk file to merge with
        if (r_disk_files.size() == 1) {
            return;
        }
    }
    // else, add memstore stream
    else {
        m_memstore->m_inputstream->reset();
        istreams_to_merge.push_back(m_memstore->m_inputstream);
    }

    // add all disk input streams that will be merged
    for (int i = 0; i < (int)r_disk_istreams.size(); i++) {
        r_disk_istreams[i]->reset();
        istreams_to_merge.push_back(r_disk_istreams[i]);
    }

    // merge input streams, writing output to a new file
    disk_file = merge_streams(istreams_to_merge);

    // clear memstore if needed
    if (get_memstore_merge_type() == CM_MERGE_ONLINE) {
        memstore_clear();
    }

    // delete all files merged as well as their input streams
    for (int i = 0; i < (int)r_disk_files.size(); i++) {
        r_disk_files[i]->delete_from_disk();
        delete r_disk_files[i];
        delete r_disk_istreams[i];
    }
    r_disk_files.clear();
    r_disk_istreams.clear();

    // add new file and its input stream to (currently empty) set of disk files
    r_disk_files.push_back(disk_file);
    r_disk_istreams.push_back(new KVTDiskFileInputStream(disk_file, MERGE_BUFSIZE));

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
