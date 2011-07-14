#ifndef IMM_COMPACTIONMANAGER_H
#define IMM_COMPACTIONMANAGER_H

#include "GeomCompactionManager.h"

class MemStore;
class DiskStore;

class ImmCompactionManager: public GeomCompactionManager {

public:

    /**
     * constructor
     */
    ImmCompactionManager(MemStore *memstore, DiskStore *diskstore);

    /**
     * destructor
     */
    ~ImmCompactionManager();
};

#endif
