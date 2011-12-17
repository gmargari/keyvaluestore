// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#include "./Global.h"
#include "./Streams.h"

#include <vector>

#include "./DiskFile.h"
#include "./DiskFileOutputStream.h"
#include "./PriorityInputStream.h"
#include "./Buffer.h"
#include "./Statistics.h"

/*============================================================================
 *                                 copy_stream
 *============================================================================*/
void Streams::copy_stream(InputStream *istream, OutputStream *ostream) {
    Slice key, value;
    uint64_t timestamp;

    time_start(&(g_stats.merge_time));

    while (istream->read(&key, &value, &timestamp)) {
        ostream->append(key, value, timestamp);
    }
    ostream->close();

    time_end(&(g_stats.merge_time));
}

/*============================================================================
 *                           copy_stream_unique_keys
 *============================================================================*/
void Streams::copy_stream_unique_keys(InputStream *istream,
                                      OutputStream *ostream) {
    Slice key, value;
    uint64_t timestamp;
    char prev_key[MAX_KVTSIZE];

    time_start(&(g_stats.merge_time));

    prev_key[0] = '\0';
    while (istream->read(&key, &value, &timestamp)) {
        if (strcmp(prev_key, key.data()) != 0) {
            ostream->append(key, value, timestamp);
            memcpy(prev_key, key.data(), key.size() + 1);  // TODO: avoid this?
        }
    }
    ostream->close();

    time_end(&(g_stats.merge_time));
}

/*============================================================================
 *                               merge_streams
 *============================================================================*/
void Streams::merge_streams(vector<InputStream *> istreams,
                            OutputStream *ostream) {
    PriorityInputStream *pistream;

    pistream = new PriorityInputStream(istreams);
    copy_stream_unique_keys(pistream, ostream);
    delete pistream;
}

/*============================================================================
 *                               merge_streams
 *============================================================================*/
void Streams::merge_streams(vector<InputStream *> istreams,
                            DiskFile *diskfile) {
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
void Streams::merge_streams(vector<InputStream *> istreams,
                            vector<DiskFile *> *diskfiles) {
    DiskFile *diskfile;

    diskfile = merge_streams(istreams);
    diskfiles->push_back(diskfile);
}

/*============================================================================
 *                               merge_streams
 *============================================================================*/
int Streams::merge_streams(vector<InputStream *> istreams,
                           vector<DiskFile *> *diskfiles,
                           uint64_t max_file_size) {
    DiskFile *diskfile;
    PriorityInputStream *pistream;
    DiskFileOutputStream *ostream;
    Slice key, value;
    char prev_key[MAX_KVTSIZE];
    uint64_t timestamp, filesize;
    uint32_t len;
    int num_newfiles;

    diskfile = new DiskFile;
    diskfile->open_new_unique();
    ostream = new DiskFileOutputStream(diskfile, MERGE_BUFSIZE);

    pistream = new PriorityInputStream(istreams);

    num_newfiles = 0;
    prev_key[0] = '\0';
    filesize = 0;

    time_start(&(g_stats.merge_time));

    while (pistream->read(&key, &value, &timestamp)) {
        if (strcmp(prev_key, key.data()) != 0) {
            // if we appending current tuple to file will lead to file size
            // > 'max_file_size', crete a new file for remaining tuples
            len = Buffer::serialize_len(key.size(), value.size(), timestamp);
            if (filesize + len > max_file_size) {
                ostream->close();
                diskfiles->push_back(diskfile);
                num_newfiles++;

                diskfile = new DiskFile;
                diskfile->open_new_unique();
                delete ostream;
                ostream = new DiskFileOutputStream(diskfile, MERGE_BUFSIZE);
                filesize = 0;
            }

            filesize += len;
            ostream->append(key, value, timestamp);
            memcpy(prev_key, key.data(), key.size() + 1);
        }
    }
    ostream->close();

    time_end(&(g_stats.merge_time));

    diskfiles->push_back(diskfile);
    num_newfiles++;

    delete pistream;
    delete ostream;

    return num_newfiles;
}
