#ifndef STREAMS_H
#define STREAMS_H

#include <stdint.h>
#include <vector>

using std::vector;

class InputStream;
class OutputStream;
class DiskFile;

class Streams {

public:

    /**
     * copy all <k,v> pairs of input stream to output stream
     */
    static void copy_stream(InputStream *istream, OutputStream *ostream);

    /**
     * copy all <k,v> pairs of input stream with unique keys to output stream
     */
    static void copy_stream_unique_keys(InputStream *istream, OutputStream *ostream);

    /**
     * merge all input streams producing one sorted output stream written to 'ostream'
     */
    static void merge_streams(vector<InputStream *> istreams, OutputStream *ostream);

    /**
     * same as function above, but takes as second argument a file instead of
     * an output stream
     */
    static void merge_streams(vector<InputStream *> istreams, DiskFile *diskfile);

    /**
     * same as function above, but it is left up to the function to create the
     * file for output and return a pointer to it
     */
    static DiskFile *merge_streams(vector<InputStream *> istreams);

    /**
     * same as function above, but a pointer to new file will be appended to
     * vector 'diskfiles'
     */
    static void merge_streams(vector<InputStream *> istreams, vector<DiskFile *>& diskfiles);

    /**
     * merge all input streams creating one or more diskfiles. each disk file
     * will have size at most 'max_file_size'. pointers to new files created
     * are appended to vector 'diskfiles'.
     *
     * @return number of new files created
     */
    static int merge_streams(vector<InputStream *> istreams, vector<DiskFile *>& diskfiles, uint64_t max_file_size);
};

#endif