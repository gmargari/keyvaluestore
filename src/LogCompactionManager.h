#ifndef LOG_COMPACTIONMANAGER_H
#define LOG_COMPACTIONMANAGER_H

#include "GeomCompactionManager.h"

class LogCompactionManager: public GeomCompactionManager {

public:

    /**
     * constructor
     */
    LogCompactionManager(MemStore *memstore, DiskStore *diskstore)
        : GeomCompactionManager(memstore, diskstore) {
        GeomCompactionManager::set_R(2);
    }

    /**
     * destructor
     */
    ~LogCompactionManager() { }
};

#endif
