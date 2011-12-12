#ifndef COMPACTIONMANAGER_H
#define COMPACTIONMANAGER_H

class MemStore;
class DiskStore;
class DiskFile;

class CompactionManager {

public:

    /**
     * constructor
     */
    CompactionManager(MemStore *memstore, DiskStore *diskstore);

    /**
     * destructor
     */
    virtual ~CompactionManager();

    /**
     * flush memstore to disk, creating a new disk file. return pointer to new
     * disk file created
     */
    DiskFile *memstore_flush_to_diskfile();

    /**
     * clear memstore
     */
    void memstore_clear();

    /**
     * flush <k,v> pairs from memory to disk, creating free space for new pairs
     */
    virtual void flush_bytes() = 0;

    // Undefined methods (just remove Weffc++ warning)
    CompactionManager(const CompactionManager&);
    CompactionManager& operator=(const CompactionManager&);

protected:

    MemStore    *m_memstore;
    DiskStore   *m_diskstore;
};

#endif
