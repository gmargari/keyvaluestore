#ifndef NULL_COMPACTIONMANAGER_H
#define NULL_COMPACTIONMANAGER_H

#include "CompactionManager.h"

class MemStore;
class DiskStore;

class NullCompactionManager: public CompactionManager {

public:

    /**
     * constructor
     */
    NullCompactionManager(MemStore *memstore, DiskStore *diskstore);

    /**
     * destructor
     */
    ~NullCompactionManager();

    // inherited from CompactionManager (see CompactionManager.h for description)
    void flush_bytes(void);

protected:

    int sanity_check();
};

#endif
