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
    typedef enum { NOMERGE_CM, IMM_CM, GEOM_CM, LOG_CM, RNGMERGE_CM  } cm_type;

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
     * set/get the type of memstore merging. if merge type is 'offline', then
     * in order to merge memstore with disk files we first flush memstore to
     * disk to a new file and then merge this new file with other disk files.
     * if merge is 'online' we don't flush memstore to disk, but merge it
     * online with disk files.
     */
    void       set_memstore_merge_type(merge_type type);
    merge_type get_memstore_merge_type();

    /**
     * function used to get statistics about keyvaluestore
     */

    uint32_t get_mb_read();
    uint32_t get_mb_written();
    uint32_t get_num_reads();
    uint32_t get_num_writes();
    // see Statistics.h for explanation of these times
    uint32_t get_total_time_sec();
    uint32_t get_put_time_sec();
    uint32_t get_compaction_time_sec();
    uint32_t get_merge_time_sec();
    uint32_t get_free_time_sec();
    uint32_t get_cmrest_time_sec();
    uint32_t get_mem_time_sec();
    uint32_t get_io_time_sec();
    uint32_t get_read_time_sec();
    uint32_t get_write_time_sec();

protected:
    void check_parameters();

    MemStore            *m_memstore;
    DiskStore           *m_diskstore;
    CompactionManager   *m_compactionmanager;
};

#endif
