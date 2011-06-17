#ifndef LOG_COMPACTIONMANAGER_H
#define LOG_COMPACTIONMANAGER_H

#include "CompactionManager.h"

#include <vector>

using std::vector;

class MemStore;
class DiskStore;

class LogCompactionManager: public CompactionManager {

public:

    /**
     * constructor
     */
    LogCompactionManager(MemStore *memstore, DiskStore *diskstore);

    /**
     * destructor
     */
    ~LogCompactionManager();

    // inherited from CompactionManager (see CompactionManager.h for description)
    void flush_bytes(void);

protected:

    int sanity_check();
};

#endif
