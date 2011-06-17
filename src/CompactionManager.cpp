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


/*========================================================================
 *                    memstore_flush_to_new_diskfile
 *========================================================================*/
void CompactionManager::memstore_flush_to_new_diskfile()
{
    KVTDiskFile *disk_file;
    KVTMapInputStream *map_istream;
    KVTDiskFileInputStream *disk_istream;
    KVTDiskFileOutputStream *disk_ostream;

    // create an input stream for memstore
    map_istream = new KVTMapInputStream(m_memstore->m_kvtmap);

    // write memstore to a new file on disk using streams
    disk_file = new KVTDiskFile;
    disk_file->open_unique();
    disk_ostream = new KVTDiskFileOutputStream(disk_file);
    // (no need to use copy_stream_unique_keys() since map keys are unique)
    copy_stream(map_istream, disk_ostream);

    // append at diskstore files vector the new disk file & a new input stream for it
    m_diskstore->m_disk_files.push_back(disk_file);
    disk_istream = new KVTDiskFileInputStream(m_diskstore->m_disk_files.back());
    m_diskstore->m_disk_istreams.push_back(disk_istream);

    // delete map input stream and disk file output stream
    delete map_istream;
    delete disk_ostream;
}

/*========================================================================
 *                           memstore_clear
 *========================================================================*/
void CompactionManager::memstore_clear()
{
    m_memstore->clear();
}