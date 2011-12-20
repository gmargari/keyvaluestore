// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_STREAMS_H_
#define SRC_STREAMS_H_

#include <stdint.h>
#include <vector>

#include "./Global.h"

using std::vector;

class InputStream;
class OutputStream;
class DiskFile;

class Streams {
  public:
    /**
     * copy all KVs of input stream to output stream
     */
    static void copy_stream(InputStream *istream, OutputStream *ostream);

    /**
     * copy all KVs of input stream with unique keys to output stream
     */
    static void copy_stream_unique_keys(InputStream *istream,
                                        OutputStream *ostream);

    /**
     * merge all input streams producing one sorted output ('ostream')
     */
    static void merge_streams(vector<InputStream *> istreams,
                              OutputStream *ostream);

    /**
     * same as function above, but takes as second argument a file instead of
     * an output stream
     */
    static void merge_streams(vector<InputStream *> istreams,
                              DiskFile *diskfile);

    /**
     * same as function above, but it is left up to the function to create the
     * file for output and return a pointer to it
     */
    static DiskFile *merge_streams(vector<InputStream *> istreams);

    /**
     * merge all input streams creating one or more diskfiles. each disk file
     * will have size at most 'max_file_size'. pointers to new files created
     * are appended to vector 'diskfiles'.
     *
     * @return number of new files created
     */
    static int merge_streams(vector<InputStream *> istreams,
                             vector<DiskFile *> *diskfiles,
                             uint64_t max_file_size);
};

#endif  // SRC_STREAMS_H_
