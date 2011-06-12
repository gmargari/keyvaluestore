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
     * insert a <key, value, timestamp> tuple into memstore. copies of the key
     * and value are created and inserted into memstore.
     *
     * @param key key to be inserted
     * @param value value to be inserted
     * @param timestamp timestamp to be inserted
     * @return true for success, false for failure
     */
    bool put(const char *key, const char *value, uint64_t timestamp);

    /**
     * insert a <key, value> pair into memstore. copies of the key and value are
     * created and inserted into memstore.
     *
     * @param key key to be inserted
     * @param value value to inserted
     * @return true for success, false for failure
     */
    bool put(const char *key, const char *value);

    /**
     * get the (copy of) value for a specific key.
     *
     * @param key (in) key to be searched
     * @param value (out) value corresponding to the searched key
     * @param timestamp (out) timestamp of insertion
     * @return true if key was found, false if not
     */
    bool get(const char *key, char **value, uint64_t *timestamp);

    /**
     * get the (copy of) value for a specific <key, timestamp> pair
     *
     * @param key (in) key to be searched
     * @param timestamp (in) timestamp of insertion
     * @param value (out) value corresponding to the searched <key, timestamp>
     * @return true if <key, timestamp> was found, false if not
     */
    bool get(const char *key, uint64_t timestamp, char **value);

    /**
     * number of <key, value, timestamp> tuples in memstore
     *
     * @return number of <key, value, timestamp> tuples in memstore
     */
    uint64_t num_keys();

    /**
     * total byte size of tuples stored in memstore
     *
     * @return total byte size of tuples stored in memstore
     */
    uint64_t size();

    /**
     * memstore is full (we probably need to flush some <k,v,t> tuples to disk)
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
