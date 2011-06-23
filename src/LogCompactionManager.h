#ifndef LOG_COMPACTIONMANAGER_H
#define LOG_COMPACTIONMANAGER_H

#include "GeomCompactionManager.h"

class MemStore;
class DiskStore;

class LogCompactionManager: public GeomCompactionManager {

public:

    /**
     * constructor
     */
    LogCompactionManager(MemStore *memstore, DiskStore *diskstore);

    /**
     * destructor
     */
    ~LogCompactionManager();
};

#endif
