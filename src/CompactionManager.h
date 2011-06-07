#ifndef COMPACTIONMANAGER_H
#define COMPACTIONMANAGER_H

#include "Global.h"
#include "MemStore.h"
#include "DiskStore.h"
#include "KVMapInputStream.h"
#include "KVDiskFileOutputStream.h"

using std::vector;

class CompactionManager {

public:

    /**
     * constructor
     */
    CompactionManager(MemStore *memstore, DiskStore *diskstore);

    /**
     * destructor
     */
    ~CompactionManager();

    /**
     * sort memory <k,v> pairs by key and store them in a new disk file
     */
    void flush_memstore(void);

    /**
     * copy all <k,v> pairs of input stream to output stream
     */
    void copy_stream(KVInputStream *istream, KVOutputStream *ostream);
    
    /**
     * merge all input streams producing one sorted output stream written to 'ostream'
     */
    void merge_streams(vector<KVInputStream *> istreams, KVOutputStream *ostream);

protected:

    void sanity_check();

    MemStore    *m_memstore;
    DiskStore   *m_diskstore;
};

#endif
