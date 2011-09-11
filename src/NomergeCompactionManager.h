#ifndef NULL_COMPACTIONMANAGER_H
#define NULL_COMPACTIONMANAGER_H

#include "CompactionManager.h"

class MemStore;
class DiskStore;

class NomergeCompactionManager: public CompactionManager {

public:

    /**
     * constructor
     */
    NomergeCompactionManager(MemStore *memstore, DiskStore *diskstore);

    /**
     * destructor
     */
    ~NomergeCompactionManager();

    // inherited from CompactionManager (see CompactionManager.h for description)
    void flush_bytes();

protected:

    int sanity_check();
};

#endif
