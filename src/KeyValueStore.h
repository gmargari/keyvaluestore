#ifndef KEYVALUESTORE_H
#define KEYVALUESTORE_H

#include <stdint.h>

class MemStore;
class DiskStore;
class CompactionManager;

class KeyValueStore {

public:

    /**
     * Memory flush & compaction manager algorithm:
     *  - IMM_CM: Immediate Merge
     *  - GEOM_CM: Geometric Partitioning
     *  - LOG_CM: Logarithmic Merge
     *  - URF_CM: Unified Range Flush
     */
    typedef enum { GEOM_CM, LOG_CM, URF_CM, IMM_CM } cm_type;

    /**
     * constructor
     */
    KeyValueStore(cm_type type);

    /**
     * destructor
     */
    ~KeyValueStore();

    /**
     * set/get maximum byte size of memstore. when this limit is reached,
     * compaction manager is called to flush some bytes from memory to disk.
     */
    void     set_memstore_maxsize(uint64_t maxsize);
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
     * return number of <key, value, timestamp> tuples in memory
     */
    uint64_t get_num_mem_keys();

    /**
     * return number of <key, value, timestamp> tuples on disk
     */
    uint64_t get_num_disk_keys();

    /**
     * return total byte size of tuples stored in memory
     */
    uint64_t get_mem_size();

    /**
     * return total byte size of tuples stored on disk
     */
    uint64_t get_disk_size();

    /**
     * return number of disk files
     */
    int get_num_disk_files();

    /**
     * explicitly call compaction manager's flush_bytes()
     */
    void flush_bytes();

    /**
     * return pointer to compaction manager, in case e.g. we want to modify
     * its parameters.
     */
    CompactionManager *get_compaction_manager();

protected:

    int sanity_check();
    void check_parameters();

    MemStore            *m_memstore;
    DiskStore           *m_diskstore;
    CompactionManager   *m_compactionmanager;
};

#endif
