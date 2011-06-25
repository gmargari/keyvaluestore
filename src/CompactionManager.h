#ifndef COMPACTIONMANAGER_H
#define COMPACTIONMANAGER_H

#include <vector>

using std::vector;

class MemStore;
class DiskStore;
class KVTInputStream;
class KVTOutputStream;

typedef enum { CM_MERGE_ONLINE, CM_MERGE_OFFLINE } merge_type;

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
    void copy_stream(KVTInputStream *istream, KVTOutputStream *ostream);

    /**
     * copy all <k,v> pairs of input stream with unique keys to output stream
     */
    void copy_stream_unique_keys(KVTInputStream *istream, KVTOutputStream *ostream);

    /**
     * merge all input streams producing one sorted output stream written to 'ostream'
     */
    void merge_streams(vector<KVTInputStream *> istreams, KVTOutputStream *ostream);
// TODO: move these to something like StreamFunctions

    /**
     * flush memstore to disk, creating a new disk file (and a corresponding
     * input stream) that is appended at the end of diskstore files vector.
     */
    void memstore_flush_to_new_diskfile();

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
