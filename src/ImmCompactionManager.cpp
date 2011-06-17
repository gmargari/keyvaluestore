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

#define MERGE_ONLINE 1

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
    KVTMapInputStream *map_istream;
    KVTDiskFileInputStream *disk_istream;
    KVTDiskFileOutputStream *disk_ostream;
    vector<KVTInputStream *> disk_istreams;
    int memstore_flushed = 0;

    // -------------------------------- NOTE ----------------------------------
    // every new file created by Compaction Manager should be inserted
    // at the *back* of the vector, so the last file in vector is the most
    // recent one, and the first is the oldest one.
    // the same should be done accordingly for their respective input streams
    // -------------------------------- NOTE ----------------------------------

    sanity_check();

    // if we dont perform online merge, or we perform online merge but
    // this is the first time memory gets full:
    if ((MERGE_ONLINE == 0) || (m_diskstore->m_disk_files.size() == 0)) {
        memstore_flush_to_new_diskfile();
        memstore_clear();
        memstore_flushed = 1;
    }

    // if we dont perform online merge, or we perform online merge but
    // we didn't flush memstore to disk above (there is an existing disk index):
    assert(m_diskstore->m_disk_istreams.size() <= 2);
    if ((MERGE_ONLINE == 0) || (memstore_flushed == 0)) {

        // create vector of all input streams that will be merged
        for (int i = 0; i < (int)m_diskstore->m_disk_istreams.size(); i++) {
            disk_istreams.push_back(m_diskstore->m_disk_istreams[i]);
        }
        if (MERGE_ONLINE == 1) {
            // create an input stream for memstore and add it to vector of
            // streams that will be merged
            map_istream = new KVTMapInputStream(m_memstore->m_kvtmap);
            disk_istreams.push_back(map_istream);
        }

        // merge input streams, writing output to a new file
        disk_file = new KVTDiskFile;
        disk_file->open_unique();
        disk_ostream = new KVTDiskFileOutputStream(disk_file);
        merge_streams(disk_istreams, disk_ostream);
        delete disk_ostream;

        // delete all files merged, as well as their input streams
        for (int i = 0; i < (int)m_diskstore->m_disk_files.size(); i++) {
            m_diskstore->m_disk_files[i]->delete_from_disk();
            delete m_diskstore->m_disk_files[i];
            delete m_diskstore->m_disk_istreams[i];
        }
        m_diskstore->m_disk_files.clear();
        m_diskstore->m_disk_istreams.clear();

        // add new file to (currently empty) set of disk files
        m_diskstore->m_disk_files.push_back(disk_file);
        disk_istream = new KVTDiskFileInputStream(m_diskstore->m_disk_files.back());
        m_diskstore->m_disk_istreams.push_back(disk_istream);

        if (MERGE_ONLINE == 1) {
            m_memstore->clear();
            delete map_istream;
        }
    }

    sanity_check();
}

/*=======================================================================*
 *                              sanity_check
 *=======================================================================*/
void ImmCompactionManager::sanity_check()
{
    return_if_dbglvl_lt_2();

    assert(m_diskstore->m_disk_files.size() == m_diskstore->m_disk_istreams.size());
}
