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
     * merge all input streams producing one sorted output stream written to 'ostream'
     */
    void merge_istreams(vector<KVInputStream *> istreams, KVDiskFileOutputStream *ostream);

    // TEST
    void catdiskfiles();
    // TEST
    void check_disk_files_are_sorted();
    
protected:

    void sanity_check();
    
    MemStore    *m_memstore;
    DiskStore   *m_diskstore;
};

#endif
