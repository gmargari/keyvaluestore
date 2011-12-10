#ifndef KEYVALUESTORE_H
#define KEYVALUESTORE_H

#include <stdint.h>

class MemStore;
class DiskStore;
class CompactionManager;

class KeyValueStore {

friend class Scanner;

public:

    /**
     * Memory flush & compaction manager algorithm:
     *  - NOMERGE_CM:  No Merge compaction manager
     *  - IMM_CM:      Immediate Merge compaction manager
     *  - GEOM_CM:     Geometric Partitioning compaction manager
     *  - LOG_CM:      Logarithmic Merge compaction manager
     *  - RNGMERGE_CM: Range Merge compaction manager
     */
    typedef enum { NOMERGE_CM, IMM_CM, GEOM_CM, LOG_CM, RNGMERGE_CM,
                   CASSANDRA_CM } cm_type;

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
     * @param keylen size of key
     * @param value value to be inserted
     * @param valuelen size of value
     * @param timestamp timestamp to be inserted
     * @return true for success, false for failure
     */
    bool put(const char *key, uint32_t keylen, const char *value, uint32_t valuelen, uint64_t timestamp);

    /**
     * insert a <key, value> tuple into store. copies of the key and value are
     * created and inserted into the store.
     *
     * @param key key to be inserted
     * @param keylen size of key
     * @param value value to be inserted
     * @param valuelen size of value
     * @return true for success, false for failure
     */
    bool put(const char *key, uint32_t keylen, const char *value, uint32_t valuelen);

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

    /**
     * return current timestamp
     */
    uint64_t get_timestamp();

    // Undefined methods (just remove Weffc++ warning)
    KeyValueStore(const KeyValueStore&);
    KeyValueStore& operator=(const KeyValueStore&);

protected:
    void check_parameters();

    MemStore            *m_memstore;
    DiskStore           *m_diskstore;
    CompactionManager   *m_compactionmanager;
};

#endif
