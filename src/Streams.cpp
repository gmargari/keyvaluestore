#include "Global.h"
#include "Streams.h"

#include "DiskFile.h"
#include "DiskFileOutputStream.h"
#include "PriorityInputStream.h"
#include "Buffer.h"
#include "Statistics.h"

/*============================================================================
 *                                 copy_stream
 *============================================================================*/
void Streams::copy_stream(InputStream *istream, OutputStream *ostream) {
    const char *key, *value;
    uint32_t keylen, valuelen;
    uint64_t timestamp;

    time_start(&(g_stats.merge_time));

    while (istream->read(&key, &keylen, &value, &valuelen, &timestamp)) {
        ostream->append(key, keylen, value, valuelen, timestamp);
    }
    ostream->close();

    time_end(&(g_stats.merge_time));
}

/*============================================================================
 *                           copy_stream_unique_keys
 *============================================================================*/
void Streams::copy_stream_unique_keys(InputStream *istream, OutputStream *ostream) {
    const char *key, *value;
    uint32_t keylen, valuelen;
    uint64_t timestamp;
    char prev_key[MAX_KVTSIZE];

    time_start(&(g_stats.merge_time));

    prev_key[0] = '\0';
    while (istream->read(&key, &keylen, &value, &valuelen, &timestamp)) {
        if (strcmp(prev_key, key) != 0) {
            ostream->append(key, keylen, value, valuelen, timestamp);
            memcpy(prev_key, key, keylen + 1);
        }
    }
    ostream->close();

    time_end(&(g_stats.merge_time));
}

/*============================================================================
 *                               merge_streams
 *============================================================================*/
void Streams::merge_streams(vector<InputStream *> istreams, OutputStream *ostream) {
    PriorityInputStream *pistream;

    pistream = new PriorityInputStream(istreams);
    copy_stream_unique_keys(pistream, ostream);
    delete pistream;
}

/*============================================================================
 *                               merge_streams
 *============================================================================*/
void Streams::merge_streams(vector<InputStream *> istreams, DiskFile *diskfile) {
    DiskFileOutputStream *ostream;

    ostream = new DiskFileOutputStream(diskfile, MERGE_BUFSIZE);
    merge_streams(istreams, ostream);
    delete ostream;
}

/*============================================================================
 *                               merge_streams
 *============================================================================*/
DiskFile *Streams::merge_streams(vector<InputStream *> istreams) {
    DiskFile *diskfile;

    diskfile = new DiskFile;
    diskfile->open_new_unique();
    merge_streams(istreams, diskfile);

    return diskfile;
}

/*============================================================================
 *                               merge_streams
 *============================================================================*/
void Streams::merge_streams(vector<InputStream *> istreams, vector<DiskFile *>& diskfiles) {
    DiskFile *diskfile;

    diskfile = merge_streams(istreams);
    diskfiles.push_back(diskfile);
}

/*============================================================================
 *                               merge_streams
 *============================================================================*/
int Streams::merge_streams(vector<InputStream *> istreams, vector<DiskFile *>& diskfiles, uint64_t max_file_size) {
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
    diskfile->open_new_unique();
    ostream = new DiskFileOutputStream(diskfile, MERGE_BUFSIZE);

    pistream = new PriorityInputStream(istreams);

    num_newfiles = 0;
    prev_key[0] = '\0';
    filesize = 0;

    time_start(&(g_stats.merge_time));

    while (pistream->read(&key, &keylen, &value, &valuelen, &timestamp)) {
        if (strcmp(prev_key, key) != 0) {
            // if we appending current tuple to file will lead to a file size
            // greater than 'max_file_size', crete a new file for remaining tuples
            len = Buffer::serialize_len(keylen, valuelen, timestamp);
            if (filesize + len > max_file_size) {
                ostream->close();
                diskfiles.push_back(diskfile);
                num_newfiles++;

                diskfile = new DiskFile;
                diskfile->open_new_unique();
                delete ostream;
                ostream = new DiskFileOutputStream(diskfile, MERGE_BUFSIZE);
                filesize = 0;
            }

            filesize += len;
            ostream->append(key, keylen, value, valuelen, timestamp);
            memcpy(prev_key, key, keylen + 1);
        }
    }
    ostream->close();

    time_end(&(g_stats.merge_time));

    diskfiles.push_back(diskfile);
    num_newfiles++;

    delete pistream;
    delete ostream;

    return num_newfiles;
}
