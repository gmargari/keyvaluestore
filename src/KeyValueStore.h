#ifndef KEYVALUESTORE_H
#define KEYVALUESTORE_H

#include "MemStore.h"
#include "DiskStore.h"
#include "CompactionManager.h"

class KeyValueStore {
   
public:

    /**
     * constructor
     */
    KeyValueStore();

    /**
     * destructor
     */
    ~KeyValueStore();

    /**
     * insert a <key, value> pair into map. copies of the key and value are
     * created and inserted into the map.
     *
     * @param key key to be inserted
     * @param value value to inserted
     * @return true for success, false for failure
     */
    bool put(const char *key, const char *value);

    /**
     * get the value for a specific key
     *
     * @param key key to be searched
     * @return pointer to value of specified key, NULL if key does not exist in
     * map (return value is not copied, caller must copy it)
     */
    char *get(char *key);

    /**
     * number of <k,v> pairs in memory
     *
     * @return number of <key, value> pairs in memory
     */
    uint64_t num_mem_keys();

    /**
     * number of <k,v> pairs on disk
     * 
     * @return number of <key, value> pairs on disk
     */
    uint64_t num_disk_keys();
    
    /**
     * byte size of memstore
     *
     * @return byte size of <key, value> pairs stored in memory
     */
    uint64_t mem_size();

    /**
     * byte size of diskstore
     *
     * @return byte size of <key, value> pairs stored on disk
     */
    uint64_t disk_size();

    // TEST
    void dumpmem();
    
protected:

    DiskStore           *m_diskstore;
    MemStore            *m_memstore;
    CompactionManager   *m_compactionmanager;
};

#endif
