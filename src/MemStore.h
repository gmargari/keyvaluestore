#ifndef MEMSTORE_H
#define MEMSTORE_H

#include <stdint.h>
#include <map>

using std::pair;

class Map;
class MapInputStream;

class MemStore {

friend class CompactionManager;
friend class NomergeCompactionManager;
friend class ImmCompactionManager;
friend class GeomCompactionManager;
friend class LogCompactionManager;
friend class RangemergeCompactionManager;
friend class Scanner;

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
    uint64_t get_maxsize();

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
     * get the (copy of) value for a specific key.
     *
     * @param key (in) key to be searched
     * @param value (out) value corresponding to the searched key
     * @param timestamp (out) timestamp of insertion
     * @return true if key was found, false if not
     */
    bool get(const char *key, char **value, uint64_t *timestamp);

    /**
     * return number of <key, value, timestamp> tuples in memstore
     */
    uint64_t get_num_keys();

    /**
     * return total byte size of tuples stored in memstore.
     */
    uint64_t get_size();

    /**
     * return total byte size of tuples with keys in the range [start_key, end_key].
     * 'start_key' and 'end_key' may or may not be included, depending on
     * 'start_incl' and 'end_incl'.
     */
    uint64_t get_size(const char *start_key, const char *end_key, bool start_key_incl, bool end_key_incl);

    /**
     * return total byte size of tuples when serialized to be stored on disk.
     */
    uint64_t get_size_when_serialized();

    /**
     * return total byte size of tuples with keys in the range [start_key, end_key]
     * when serialized to be stored on disk.
     * 'start_key' and 'end_key' may or may not be included, depending on
     * 'start_incl' and 'end_incl'.
     */
    uint64_t get_size_when_serialized(const char *start_key, const char *end_key, bool start_key_incl, bool end_key_incl);

    /**
     * return both the byte size of memory tuples and the byte size of
     * memory tuples when serialized
     */
    pair<uint64_t, uint64_t> get_sizes();

    /**
     * similar to function above, but caller can specify range of keys
     */
    pair<uint64_t, uint64_t> get_sizes(const char *start_key, const char *end_key, bool start_key_incl, bool end_key_incl);

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
     * clear all elements of memstore with keys in the range [start_key, end_key].
     * 'start_key' and 'end_key' may or may not be included, depending on
     * 'start_incl' and 'end_incl'.
     */
    void clear(const char *start_key, const char *end_key, bool start_key_incl, bool end_key_incl);

    // Undefined methods (just remove Weffc++ warning)
    MemStore(const MemStore&);
    MemStore& operator=(const MemStore&);

protected:

    Map      *m_map;
    uint64_t  m_maxsize;
};

#endif
