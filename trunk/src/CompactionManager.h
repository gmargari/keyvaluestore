#ifndef COMPACTIONMANAGER_H
#define COMPACTIONMANAGER_H

#include <stdint.h>
#include <vector>

using std::vector;

class MemStore;
class DiskStore;
class InputStream;
class OutputStream;
class DiskFile;

class CompactionManager {

public:

    /**
     * constructor
     */
    CompactionManager(MemStore *memstore, DiskStore *diskstore);

    /**
     * destructor
     */
    virtual ~CompactionManager();

// TODO: move these to something like StreamFunctions
    /**
     * copy all <k,v> pairs of input stream to output stream
     */
    void copy_stream(InputStream *istream, OutputStream *ostream);

    /**
     * copy all <k,v> pairs of input stream with unique keys to output stream
     */
    void copy_stream_unique_keys(InputStream *istream, OutputStream *ostream);

    /**
     * merge all input streams producing one sorted output stream written to 'ostream'
     */
    void merge_streams(vector<InputStream *> istreams, OutputStream *ostream);

    /**
     * same as function above, but takes as second argument a file instead of
     * an output stream
     */
    void merge_streams(vector<InputStream *> istreams, DiskFile *diskfile);

    /**
     * same as function above, but it is left up to the function to create the
     * file for output and return a pointer to it
     */
    DiskFile *merge_streams(vector<InputStream *> istreams);

    /**
     * same as function above, but a pointer to new file will be appended to
     * vector 'diskfiles'
     */
    void merge_streams(vector<InputStream *> istreams, vector<DiskFile *>& diskfiles);

    /**
     * merge all input streams creating one or more diskfiles. each disk file
     * will have size at most 'max_file_size'. pointers to new files created
     * are appended to vector 'diskfiles'.
     *
     * @return number of new files created
     */
    int merge_streams(vector<InputStream *> istreams, vector<DiskFile *>& diskfiles, uint64_t max_file_size);
// TODO: move these to something like StreamFunctions

    /**
     * flush memstore to disk, creating a new disk file. return pointer to new
     * disk file created
     */
    DiskFile *memstore_flush_to_diskfile();

    /**
     * clear memstore
     */
    void memstore_clear();

    /**
     * set/get the type of memstore merging. if merge type is 'offline', then
     * in order to merge memstore with disk files we first flush memstore to
     * disk to a new file and then merge this new file with other disk files.
     * if merge is 'online' we don't flush memstore to disk, but merge it
     * online with disk files.
     */
    void       set_memstore_merge_type(merge_type type);
    merge_type get_memstore_merge_type();

    /**
     * flush <k,v> pairs from memory to disk, creating free space for new pairs
     */
    virtual void flush_bytes(void) = 0;

protected:

    MemStore    *m_memstore;
    DiskStore   *m_diskstore;
    merge_type   m_merge_type;
};

#endif
