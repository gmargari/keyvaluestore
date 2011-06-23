#ifndef IMM_COMPACTIONMANAGER_H
#define IMM_COMPACTIONMANAGER_H

#include "CompactionManager.h"

class MemStore;
class DiskStore;

class ImmCompactionManager: public CompactionManager {

public:

    /**
     * constructor
     */
    ImmCompactionManager(MemStore *memstore, DiskStore *diskstore);

    /**
     * destructor
     */
    ~ImmCompactionManager();

    // inherited from CompactionManager (see CompactionManager.h for description)
    void flush_bytes(void);

protected:

    int sanity_check();
};

#endif
