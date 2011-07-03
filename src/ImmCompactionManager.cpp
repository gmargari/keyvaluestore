#include "Global.h"
#include "ImmCompactionManager.h"

#include "MemStore.h"
#include "DiskStore.h"
#include "MapInputStream.h"
#include "DiskFile.h"
#include "DiskFileInputStream.h"

#include <cassert>

/*============================================================================
 *                            ImmCompactionManager
 *============================================================================*/
ImmCompactionManager::ImmCompactionManager(MemStore *memstore, DiskStore *diskstore)
: CompactionManager(memstore, diskstore)
{

}

/*============================================================================
 *                           ~ImmCompactionManager
 *============================================================================*/
ImmCompactionManager::~ImmCompactionManager()
{

}

#include <cstdio>

/*============================================================================
 *                               flush_bytes
 *============================================================================*/
void ImmCompactionManager::flush_bytes(void)
{
    DiskFile *disk_file, *memstore_file;
    DiskFileInputStream *memstore_file_istream;
    vector<InputStream *> istreams_to_merge;
    vector<DiskFile *> &r_disk_files = m_diskstore->m_disk_files;
    vector<DiskFileInputStream *> &r_disk_istreams = m_diskstore->m_disk_istreams;

    assert(sanity_check());

    //--------------------------------------------------------------------------
    // 1) add input streams of disk file to vector of streams to merge
    //--------------------------------------------------------------------------
    if (r_disk_istreams.size()) {
        r_disk_istreams[0]->reset();
        istreams_to_merge.push_back(r_disk_istreams[0]);
    }

    //--------------------------------------------------------------------------
    // 2) add memstore stream to vector of streams to merge
    //--------------------------------------------------------------------------

    // if we perform online merge, add memstore stream to vector of streams
    if (get_memstore_merge_type() == CM_MERGE_ONLINE) {
printf("online!\n");
        m_memstore->m_inputstream->reset();
        istreams_to_merge.push_back(m_memstore->m_inputstream);
    }
    // else, flush memstore to new file, add file stream to vector of streams
    else {
        memstore_file = memstore_flush_to_diskfile();
        memstore_file_istream = new DiskFileInputStream(memstore_file, MERGE_BUFSIZE);
        istreams_to_merge.push_back(memstore_file_istream);

        memstore_clear();
    }

    //--------------------------------------------------------------------------
    // 3) merge streams creating a new disk file
    //--------------------------------------------------------------------------
    disk_file = merge_streams(istreams_to_merge);

    //--------------------------------------------------------------------------
    // 4) delete merged files and corresponding streams from DiskStore
    //--------------------------------------------------------------------------

    // if we performed online merge, clear memstore
    if (get_memstore_merge_type() == CM_MERGE_ONLINE) {
        memstore_clear();
    }
    // else, memstore already cleared, delete the memstore file and disk stream
    else {
        memstore_file->delete_from_disk();
        delete memstore_file;
        delete memstore_file_istream;
    }

    // delete all files merged as well as their input streams
    for (int i = 0; i < (int)r_disk_files.size(); i++) {
        r_disk_files[i]->delete_from_disk();
        delete r_disk_files[i];
        delete r_disk_istreams[i];
    }
    r_disk_files.clear();
    r_disk_istreams.clear();

    //--------------------------------------------------------------------------
    // 5) add new file and corresponding stream to (currently empty) DiskStore
    //--------------------------------------------------------------------------

    r_disk_files.push_back(disk_file);
    r_disk_istreams.push_back(new DiskFileInputStream(disk_file, MERGE_BUFSIZE));

    assert(sanity_check());
}

/*============================================================================
 *                                sanity_check
 *============================================================================*/
int ImmCompactionManager::sanity_check()
{
    assert(m_diskstore->m_disk_files.size() == m_diskstore->m_disk_istreams.size());
    assert(m_diskstore->m_disk_istreams.size() <= 1);

    return 1;
}
