#ifndef COMPACTIONMANAGER_H
#define COMPACTIONMANAGER_H

#include "Global.h"
#include "MemStore.h"
#include "DiskStore.h"
#include "KVTMapInputStream.h"
#include "KVTDiskFileOutputStream.h"

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
    void copy_stream(KVTInputStream *istream, KVTOutputStream *ostream);

    /**
     * merge all input streams producing one sorted output stream written to 'ostream'
     */
    void merge_streams(vector<KVTInputStream *> istreams, KVTOutputStream *ostream);

protected:

    void sanity_check();

    MemStore    *m_memstore;
    DiskStore   *m_diskstore;
};

#endif
