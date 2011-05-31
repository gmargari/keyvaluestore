#ifndef COMPACTIONMANAGER_H
#define COMPACTIONMANAGER_H

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

    /**
     * sort memory <k,v> pairs by key and store them in a new disk file
     */
    void memstore_to_diskfile(void);

protected:

    MemStore    *m_memstore;
    DiskStore   *m_diskstore;
};

#endif
