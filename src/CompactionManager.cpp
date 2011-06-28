#include "Global.h"
#include "CompactionManager.h"

#include "MemStore.h"
#include "DiskStore.h"
#include "MapInputStream.h"
#include "DiskFile.h"
#include "DiskFileInputStream.h"
#include "DiskFileOutputStream.h"
#include "PriorityInputStream.h"
#include "Serialization.h"

/*============================================================================
 *                              CompactionManager
 *============================================================================*/
CompactionManager::CompactionManager(MemStore *memstore, DiskStore *diskstore)
{
    m_memstore = memstore;
    m_diskstore = diskstore;
    m_merge_type = CM_MERGE_ONLINE;
}

/*============================================================================
 *                              ~CompactionManager
 *============================================================================*/
CompactionManager::~CompactionManager()
{

}

/*============================================================================
 *                                 copy_stream
 *============================================================================*/
void CompactionManager::copy_stream(InputStream *istream, OutputStream *ostream)
{
    const char *key, *value;
    uint64_t timestamp;

    while (istream->read(&key, &value, &timestamp)) {
        ostream->write(key, value, timestamp);
    }
    ostream->flush();
}

/*============================================================================
 *                           copy_stream_unique_keys
 *============================================================================*/
void CompactionManager::copy_stream_unique_keys(InputStream *istream, OutputStream *ostream)
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

/*============================================================================
 *                               merge_streams
 *============================================================================*/
void CompactionManager::merge_streams(vector<InputStream *> istreams, OutputStream *ostream)
{
    PriorityInputStream *istream_heap;

    istream_heap = new PriorityInputStream(istreams);
    copy_stream_unique_keys(istream_heap, ostream);
    delete istream_heap;
}

/*============================================================================
 *                               merge_streams
 *============================================================================*/
void CompactionManager::merge_streams(vector<InputStream *> istreams, DiskFile *diskfile)
{
    DiskFileOutputStream *ostream;

    ostream = new DiskFileOutputStream(diskfile, MERGE_BUFSIZE);
    merge_streams(istreams, ostream);
    delete ostream;
}

/*============================================================================
 *                               merge_streams
 *============================================================================*/
DiskFile *CompactionManager::merge_streams(vector<InputStream *> istreams)
{
    DiskFile *diskfile;

    diskfile = new DiskFile;
    diskfile->open_unique();
    merge_streams(istreams, diskfile);

    return diskfile;
}

/*============================================================================
 *                               merge_streams
 *============================================================================*/
void CompactionManager::merge_streams(vector<InputStream *> istreams, vector<DiskFile *>& diskfiles)
{
    DiskFile *diskfile;

    diskfile = merge_streams(istreams);
    diskfiles.push_back(diskfile);
}

/*============================================================================
 *                               merge_streams
 *============================================================================*/
int CompactionManager::merge_streams(vector<InputStream *> istreams, vector<DiskFile *>& diskfiles, uint64_t max_file_size)
{
    DiskFile *diskfile;
    PriorityInputStream *istream_heap;
    DiskFileOutputStream *ostream;
    const char *key, *value;
    char prev_key[MAX_KVTSIZE];
    uint64_t timestamp, filesize;
    uint32_t len;
    int num_newfiles;

    diskfile = new DiskFile;
    diskfile->open_unique();
    ostream = new DiskFileOutputStream(diskfile, MERGE_BUFSIZE);

    istream_heap = new PriorityInputStream(istreams);

    num_newfiles = 0;
    prev_key[0] = '\0';
    filesize = 0;
    while (istream_heap->read(&key, &value, &timestamp)) {
        if (strcmp(prev_key, key) != 0) {
            // if we appending current tuple to file will lead to a file size
            // greater than 'max_file_size', crete a new file for remaining tuples
            len = serialize_len(strlen(key), strlen(value), timestamp);
            if (filesize + len > max_file_size) {
                ostream->flush();
                diskfiles.push_back(diskfile);
                num_newfiles++;

                diskfile = new DiskFile;
                filesize = 0;
                diskfile->open_unique();
                delete ostream;
                ostream = new DiskFileOutputStream(diskfile, MERGE_BUFSIZE);
            }

            filesize += len;
            ostream->write(key, value, timestamp);
            strcpy(prev_key, key);
        }
    }
    ostream->flush();
    diskfiles.push_back(diskfile);
    num_newfiles++;

    delete istream_heap;
    delete ostream;

    return num_newfiles;
}

/*============================================================================
 *                      memstore_flush_to_new_diskfile
 *============================================================================*/
void CompactionManager::memstore_flush_to_new_diskfile()
{
    DiskFile *disk_file;
    DiskFileInputStream *disk_istream;
    DiskFileOutputStream *disk_ostream;

    // write memstore to a new file on disk using streams
    disk_file = new DiskFile();
    disk_file->open_unique();
    disk_ostream = new DiskFileOutputStream(disk_file, MERGE_BUFSIZE);
    m_memstore->m_inputstream->set_key_range(NULL, NULL);
    m_memstore->m_inputstream->reset();
    // (no need to use copy_stream_unique_keys() since map keys are unique)
    copy_stream(m_memstore->m_inputstream, disk_ostream);

    // insert first, in diskstore files vector & input streams vector, as it
    // contains the most recent <k,v> pairs
    m_diskstore->m_disk_files.insert(m_diskstore->m_disk_files.begin(), disk_file);
    disk_istream = new DiskFileInputStream(m_diskstore->m_disk_files.back(), MERGE_BUFSIZE);
    m_diskstore->m_disk_istreams.insert(m_diskstore->m_disk_istreams.begin(), disk_istream);

    // delete map input stream and disk file output stream
    delete disk_ostream;

    assert(m_memstore->get_size_when_serialized() == disk_file->get_size());
}

/*============================================================================
 *                              memstore_clear
 *============================================================================*/
void CompactionManager::memstore_clear()
{
    m_memstore->clear();
}

/*============================================================================
 *                         set_memstore_merge_type
 *============================================================================*/
void CompactionManager::set_memstore_merge_type(merge_type type)
{
    m_merge_type = type;
}

/*============================================================================
 *                         get_memstore_merge_type
 *============================================================================*/
merge_type CompactionManager::get_memstore_merge_type()
{
    return m_merge_type;
}