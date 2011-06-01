#ifndef COMPACTIONMANAGER_H
#define COMPACTIONMANAGER_H

#include "Global.h"
#include "MemStore.h"
#include "DiskStore.h"

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

    //TEST
    /**
     * sort memory <k,v> pairs by key and store them in a new disk file
     */
    void flush_memstore(void);

    // TEST
    void catdiskfiles();
    
protected:

    MemStore    *m_memstore;
    DiskStore   *m_diskstore;
};

#endif
