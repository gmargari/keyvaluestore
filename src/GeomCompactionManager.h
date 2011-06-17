#ifndef GEOM_COMPACTIONMANAGER_H
#define GEOM_COMPACTIONMANAGER_H

#include "CompactionManager.h"

#include <vector>

using std::vector;

class MemStore;
class DiskStore;

class GeomCompactionManager: public CompactionManager {

public:

    /**
     * constructor
     */
    GeomCompactionManager(MemStore *memstore, DiskStore *diskstore);

    /**
     * destructor
     */
    ~GeomCompactionManager();

    // inherited from CompactionManager (see CompactionManager.h for description)
    void flush_bytes(void);

protected:

    int sanity_check();
};

#endif
