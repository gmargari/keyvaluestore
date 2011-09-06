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
#include "Statistics.h"

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

    time_start(&(g_stats.merge_time));

    while (istream->read(&key, &value, &timestamp)) {
        ostream->write(key, strlen(key), value, strlen(value), timestamp);
    }
    ostream->flush();

    time_end(&(g_stats.merge_time));
}

/*============================================================================
 *                           copy_stream_unique_keys
 *============================================================================*/
void CompactionManager::copy_stream_unique_keys(InputStream *istream, OutputStream *ostream)
{
    const char *key, *value;
    uint64_t timestamp;
    char prev_key[MAX_KVTSIZE];

    time_start(&(g_stats.merge_time));

    prev_key[0] = '\0';
    while (istream->read(&key, &value, &timestamp)) {
        if (strcmp(prev_key, key) != 0) {
            ostream->write(key, strlen(key), value, strlen(value), timestamp);
            strcpy(prev_key, key);
        }
    }
    ostream->flush();

    time_end(&(g_stats.merge_time));
}

/*============================================================================
 *                               merge_streams
 *============================================================================*/
void CompactionManager::merge_streams(vector<InputStream *> istreams, OutputStream *ostream)
{
    PriorityInputStream *pistream;

    pistream = new PriorityInputStream(istreams);
    copy_stream_unique_keys(pistream, ostream);
    delete pistream;
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
    PriorityInputStream *pistream;
    DiskFileOutputStream *ostream;
    const char *key, *value;
    char prev_key[MAX_KVTSIZE];
    uint64_t timestamp, filesize;
    uint32_t len;
    int num_newfiles;
    uint32_t keylen, valuelen;

    diskfile = new DiskFile;
    diskfile->open_unique();
    ostream = new DiskFileOutputStream(diskfile, MERGE_BUFSIZE);

    pistream = new PriorityInputStream(istreams);

    num_newfiles = 0;
    prev_key[0] = '\0';
    filesize = 0;

    time_start(&(g_stats.merge_time));

    while (pistream->read(&key, &value, &timestamp)) {
        if (strcmp(prev_key, key) != 0) {
            keylen = strlen(key);
            valuelen = strlen(value);
            // if we appending current tuple to file will lead to a file size
            // greater than 'max_file_size', crete a new file for remaining tuples
            len = serialize_len(keylen, valuelen, timestamp);
            if (filesize + len > max_file_size) {
                ostream->flush();
                diskfiles.push_back(diskfile);
                num_newfiles++;

                diskfile = new DiskFile;
                diskfile->open_unique();
                delete ostream;
                ostream = new DiskFileOutputStream(diskfile, MERGE_BUFSIZE);
                filesize = 0;
            }

            filesize += len;
            ostream->write(key, keylen, value, valuelen, timestamp);
            strcpy(prev_key, key);
        }
    }
    ostream->flush();

    time_end(&(g_stats.merge_time));

    diskfiles.push_back(diskfile);
    num_newfiles++;

    delete pistream;
    delete ostream;

    return num_newfiles;
}

/*============================================================================
 *                        memstore_flush_to_diskfile
 *============================================================================*/
DiskFile *CompactionManager::memstore_flush_to_diskfile()
{
    DiskFile *disk_file;
    DiskFileOutputStream *disk_ostream;

    // write memstore to a new file on disk
    disk_file = new DiskFile();
    disk_file->open_unique();
    disk_ostream = new DiskFileOutputStream(disk_file, MERGE_BUFSIZE);
    m_memstore->m_inputstream->set_key_range(NULL, NULL);
    // no need to use copy_stream_unique_keys() since map keys are unique
    copy_stream(m_memstore->m_inputstream, disk_ostream);
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
