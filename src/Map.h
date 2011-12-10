#ifndef KVTMAP_H
#define KVTMAP_H

#include "slice.h"

#include <stdint.h>
#include <cstring>
#include <map>

using std::map;
using std::pair;

class Map {

friend class MapInputStream;

public:

    /**
     * constructor
     */
    Map();

    /**
     * destructor
     */
    ~Map();

    /**
     * insert a <key, value, timestamp> tuple into map. copies of the key and
     * value are created and inserted into map.
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
     * get the value for a specific key
     *
     * @param key (in) key to be searched
     * @param keylen (in) size of key
     * @param value (out) value corresponding to the searched key
     * @param valuelen (out) size of value
     * @param timestamp (out) timestamp of insertion
     * @return true if key was found, false if not
     */
    bool get(const char *key, uint32_t keylen, const char **value, uint32_t *valuelen, uint64_t *timestamp);

    /**
     * return number of <key, value, timestamp> tuples in map
     */
    uint64_t get_num_keys();

    /**
     * return total byte size of tuples stored in map.
     */
    uint64_t get_size();

    /**
     * return total byte size of tuples when serialized to be stored on disk.
     */
    uint64_t get_size_when_serialized();

    /**
     * return both the byte size of memory tuples and the byte size of
     * memory tuples when serialized
     */
    pair<uint64_t, uint64_t> get_sizes();

    /**
     * size of KV when serialized and added to map
     */
    static uint32_t kv_size(const char *key, uint32_t keylen, const char *value, uint32_t valuelen, uint64_t timestamp);

    /**
     * clear all elements of map
     */
    void clear();

    struct cmp_slice { // required, in order for the map to have its keys sorted
        bool operator()(Slice const a, Slice const b) {
            return strcmp(a.data(), b.data()) < 0;
        }
    };

    // map: key --> <value, timestamp>
    typedef pair<Slice, uint64_t > KVTPair;
    typedef map<const Slice, KVTPair, cmp_slice> KVTMap;

protected:

    int sanity_check();

    /**
     * return an iterator pointing at the first element that is equal or
     * greater than 'key' if 'key_incl' is true, or strictly greater than
     * 'key' if 'key_incl' is false.
     *
     * @param key key to be searched
     * @param key_incl whether the key should be inclusive or not
     * @return iterator at specific map element
     */
    KVTMap::iterator start_iter(const char *key, uint32_t keylen, bool key_incl);

    /**
     * return an iterator which points at the last element which equal or
     * less than 'key' if 'key_incl' is true, or strictly less than
     * 'key' if 'key_incl' is false.
     *
     * @param key key to be searched
     * @param key_incl whether the key should be inclusive or not
     * @return iterator at specific map element
     */
    KVTMap::iterator end_iter(const char *key, uint32_t keylen, bool key_incl);

    KVTMap      m_map;
    uint64_t    m_size;            // size of map
    uint64_t    m_size_serialized; // size of map when written to disk
    uint64_t    m_keys;            // number of keys or tuples in map
};

#endif
