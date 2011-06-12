#ifndef KEYVALUESTORE_H
#define KEYVALUESTORE_H

#include "Global.h"
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
     * set maximum byte size of memstore. when this limit is reached,
     * compaction manager is called to flush some bytes from memory to disk.
     */
    void set_memstore_maxsize(uint64_t maxsize);

    /**
     * get maximum byte size of memstore
     */
    uint64_t get_memstore_maxsize();

    /**
     * insert a <key, value, timestamp> tuple into store. copies of the key and
     * value are created and inserted into the store.
     *
     * @param key key to be inserted
     * @param value value to be inserted
     * @param timestamp timestamp to be inserted
     * @return true for success, false for failure
     */
    bool put(const char *key, const char *value, uint64_t timestamp);

    /**
     * insert a <key, value> tuple into store. copies of the key and value are
     * created and inserted into the store.
     *
     * @param key key to be inserted
     * @param value value to inserted
     * @return true for success, false for failure
     */
    bool put(const char *key, const char *value);

    /**
     * get the (copy of) value for a specific key
     *
     * @param key (in) key to be searched
     * @param value (out) value corresponding to the searched key
     * @param timestamp (out) timestamp of insertion
     * @return true if key was found, false if not
     */
    bool get(const char *key, char **value, uint64_t *timestamp);

    /**
     * get the (copy of) value for a specific <key, timestamp> pair.
     *
     * @param key (in) key to be searched
     * @param timestamp (in) timestamp of insertion
     * @param value (out) value corresponding to the searched <key, timestamp>
     * @return true if <key, timestamp> was found, false if not
     */
    bool get(const char *key, uint64_t timestamp, char **value);

    /**
     * number of <key, value, timestamp> tuples in memory
     *
     * @return number of <key, value, timestamp> tuples in memory
     */
    uint64_t num_mem_keys();

    /**
     * number of <key, value, timestamp> tuples on disk
     *
     * @return number of <key, value, timestamp> tuples on disk
     */
    uint64_t num_disk_keys();

    /**
     * total byte size of tuples stored in memory
     *
     * @return total byte size of tuples stored in memory
     */
    uint64_t mem_size();

    /**
     * total byte size of tuples stored on disk
     *
     * @return total byte size of tuples stored on disk
     */
    uint64_t disk_size();

protected:

    void sanity_check();
    void check_parameters();

    MemStore            *m_memstore;
    DiskStore           *m_diskstore;
    CompactionManager   *m_compactionmanager;
};

#endif
