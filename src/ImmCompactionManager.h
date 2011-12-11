#ifndef IMM_COMPACTIONMANAGER_H
#define IMM_COMPACTIONMANAGER_H

#include "GeomCompactionManager.h"

class ImmCompactionManager: public GeomCompactionManager {

public:

    /**
     * constructor
     */
    ImmCompactionManager(MemStore *memstore, DiskStore *diskstore)
        : GeomCompactionManager(memstore, diskstore) {
        GeomCompactionManager::set_P(1);
    }

    /**
     * destructor
     */
    ~ImmCompactionManager() { }
};

#endif
