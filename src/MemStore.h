#ifndef MEMSTORE_H
#define MEMSTORE_H

#include "Global.h"
#include "KVTMap.h"

class MemStore {

friend class CompactionManager;

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
     * get/set maximum size of memstore
     */
    void        set_maxsize(uint64_t maxsize);
    uint64_t    get_maxsize(void);

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
     * map
     */
    const char *get(const char *key);

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

    /**
     * memstore is full (we probably need to flush some <k,v> pairs to disk)
     */
    bool is_full();

    /**
     * clear memstore
     */
    void clear();

protected:

    void sanity_check();

    KVTMap      *m_kvtmap;
    uint64_t     m_maxsize;
};

#endif
