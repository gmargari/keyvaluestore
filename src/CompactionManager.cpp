#include "Global.h"
#include "MemStore.h"
#include "DiskStore.h"
#include "CompactionManager.h"
#include "KVTMapInputStream.h"
#include "KVTDiskFile.h"
#include "KVTDiskFileInputStream.h"
#include "KVTDiskFileOutputStream.h"
#include "KVTPriorityInputStream.h"

/*========================================================================
 *                           CompactionManager
 *========================================================================*/
CompactionManager::CompactionManager(MemStore *memstore, DiskStore *diskstore)
{
    m_memstore = memstore;
    m_diskstore = diskstore;
}

/*========================================================================
 *                          ~CompactionManager
 *========================================================================*/
CompactionManager::~CompactionManager()
{

}

/*========================================================================
 *                            flush_memstore
 *========================================================================*/
void CompactionManager::flush_memstore(void)
{
    KVTDiskFile *disk_file;
    KVTMapInputStream *map_istream;
    KVTDiskFileInputStream *disk_istream;
    KVTDiskFileOutputStream *disk_ostream;
    vector<KVTInputStream *> disk_istreams;

    // -------------------------------- NOTE ----------------------------------
    // every new file created by Compaction Manager should be inserted
    // at the *back* of the vector, so the last file in vector is the most
    // recent one, and the first is the oldest one.
    // the same should be done accordingly for their respective input streams
    // -------------------------------- NOTE ----------------------------------

    sanity_check();

    /*
     * first, flush memstore to a new file on disk
     */

    // create an input stream for memstore
    map_istream = new KVTMapInputStream(m_memstore->m_kvtmap);

    // write memstore to a new file on disk using streams, clear memstore
    disk_file = new KVTDiskFile;
    disk_file->open_unique();
    disk_ostream = new KVTDiskFileOutputStream(disk_file);
    copy_stream(map_istream, disk_ostream); // no need to use copy_stream_unique_keys() since map keys are unique
    m_memstore->clear();

    // add at diskstore new disk file & new input stream for it
    m_diskstore->m_disk_files.push_back(disk_file);
    disk_istream = new KVTDiskFileInputStream(m_diskstore->m_disk_files.back());
    m_diskstore->m_disk_istreams.push_back(disk_istream);

    delete disk_ostream;
    delete map_istream;

    /*
     * if we need to merge some disk files, merge them
     */
    if (m_diskstore->m_disk_files.size() > 3) {

        // set buffer size to maximum, to improve merge performance
        for (int i = 0; i < (int)m_diskstore->m_disk_istreams.size(); i++) {
            m_diskstore->m_disk_istreams[i]->set_buf_size_max();
        }

        // create vector of all input streams that will be merged
        for (int i = 0; i < (int)m_diskstore->m_disk_istreams.size(); i++) {
            disk_istreams.push_back(m_diskstore->m_disk_istreams[i]);
        }

        // merge input streams, writing output to a new file
        disk_file = new KVTDiskFile;
        disk_file->open_unique();
        disk_ostream = new KVTDiskFileOutputStream(disk_file);
        merge_streams(disk_istreams, disk_ostream);

        // restore buffer size, to improve search (get) performance
        for (int i = 0; i < (int)m_diskstore->m_disk_istreams.size(); i++) {
            m_diskstore->m_disk_istreams[i]->set_buf_size_min();
        }

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

        // free memory
        delete disk_ostream;
    }

    sanity_check();
}

/*========================================================================
 *                             copy_stream
 *========================================================================*/
void CompactionManager::copy_stream(KVTInputStream *istream, KVTOutputStream *ostream)
{
    const char *key, *value;
    uint64_t timestamp;

    while (istream->read(&key, &value, &timestamp)) {
        ostream->write(key, value, timestamp);
    }
    ostream->flush();
}

/*========================================================================
 *                       copy_stream_unique_keys
 *========================================================================*/
void CompactionManager::copy_stream_unique_keys(KVTInputStream *istream, KVTOutputStream *ostream)
{
    const char *key, *value;
    uint64_t timestamp;
    char prev_key[MAX_KVTSIZE];

    prev_key[0] = '\0';
    while (istream->read(&key, &value, &timestamp)) {
        if (strcmp(prev_key, key) != 0) {
            ostream->write(key, value, timestamp);
            strcpy(prev_key, key);
        }
    }
    ostream->flush();
}

/*========================================================================
 *                             merge_streams
 *========================================================================*/
void CompactionManager::merge_streams(vector<KVTInputStream *> istreams, KVTOutputStream *ostream)
{
    KVTPriorityInputStream *istream_heap;

    istream_heap = new KVTPriorityInputStream(istreams);
    copy_stream_unique_keys(istream_heap, ostream);
    delete istream_heap;
}

/*=======================================================================*
 *                              sanity_check
 *=======================================================================*/
void CompactionManager::sanity_check()
{
    return_if_dbglvl_lt_2();

    assert(m_diskstore->m_disk_files.size() == m_diskstore->m_disk_istreams.size());
}
