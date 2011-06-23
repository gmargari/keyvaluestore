#ifndef MEMSTORE_H
#define MEMSTORE_H

#include <stdint.h>

class KVTMap;
class KVTMapInputStream;

class MemStore {

friend class CompactionManager;
friend class ImmCompactionManager;
friend class GeomCompactionManager;
friend class LogCompactionManager;
friend class UrfCompactionManager;

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
    void     set_maxsize(uint64_t maxsize);
    uint64_t get_maxsize(void);

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
     * return number of <key, value, timestamp> tuples in memstore
     */
    uint64_t get_num_keys();

    /**
     * return total byte size of tuples stored in memstore.
     */
    uint64_t get_size();

    /**
     * return total byte size of tuples with keys in the range
     * ['start_key', 'end_key'), i.e. 'start_key' inclusive, 'end_key' exclusive.
     */
    uint64_t get_size(const char *start_key, const char *end_key);

    /**
     * return total byte size of tuples with keys in the range [start_key, end_key].
     * 'start_key' and 'end_key' may or may not be included, depending on
     * 'start_incl' and 'end_incl'.
     */
    uint64_t get_size(const char *start_key, const char *end_key, bool start_key_incl, bool end_key_incl);

    /**
     * check if adding <key, value, timestamp> to memstore will reach memstore's
     * size limit
     */
    bool will_reach_size_limit(const char *key, const char *value, uint64_t timestamp);

    bool will_reach_size_limit(const char *key, const char *value);

    /**
     * clear memstore
     */
    void clear();

    /**
     * clear all elements of memstore with keys in the range ['start_key', 'end_key')
     * i.e. 'start_key' inclusive, 'end_key' exclusive
     */
    void clear(const char *start_key, const char *end_key);

    /**
     * clear all elements of memstore with keys in the range [start_key, end_key].
     * 'start_key' and 'end_key' may or may not be included, depending on
     * 'start_incl' and 'end_incl'.
     */
    void clear(const char *start_key, const char *end_key, bool start_key_incl, bool end_key_incl);

protected:

    int sanity_check();

    KVTMap              *m_kvtmap;
    uint64_t             m_maxsize;
    KVTMapInputStream   *m_inputstream;
};

#endif
