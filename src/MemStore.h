#ifndef MEMSTORE_H
#define MEMSTORE_H

#include "KVMap.h"

class MemStore {
   
public:

    /**
     * constructor
     */
    MemStore();

    /**
     * destructor
     */
    ~MemStore();

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
     * number of <key, value> pairs in memstore
     *
     * @return number of <key, value> pairs in memstore
     */
    uint64_t num_keys();

    /**
     * byte size of memstore
     *
     * @return byte size of <key, value> pairs in memstore
     */
    uint64_t size();

    friend class CompactionManager;
    
protected:

    KVMap m_kvmap;

};

#endif
