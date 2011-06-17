#ifndef URF_COMPACTIONMANAGER_H
#define URF_COMPACTIONMANAGER_H

#include "CompactionManager.h"

#include <vector>

using std::vector;

class MemStore;
class DiskStore;

class UrfCompactionManager: public CompactionManager {

public:

    /**
     * constructor
     */
    UrfCompactionManager(MemStore *memstore, DiskStore *diskstore);

    /**
     * destructor
     */
    ~UrfCompactionManager();

    // inherited from CompactionManager (see CompactionManager.h for description)
    void flush_bytes(void);

protected:

    int sanity_check();
};

#endif
