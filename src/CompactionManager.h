#ifndef COMPACTIONMANAGER_H
#define COMPACTIONMANAGER_H

#include <stdint.h>
#include <vector>

using std::vector;

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
     * set/get the type of memstore merging. if merge type is 'offline', then
     * in order to merge memstore with disk files we first flush memstore to
     * disk to a new file and then merge this new file with other disk files.
     * if merge is 'online' we don't flush memstore to disk, but merge it
     * online with disk files.
     */
    void       set_memstore_merge_type(merge_type type);
    merge_type get_memstore_merge_type();

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
    merge_type   m_merge_type;
};

#endif
