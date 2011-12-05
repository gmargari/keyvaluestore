#ifndef KVTMAP_H
#define KVTMAP_H

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
     * @param value value to be inserted
     * @param timestamp timestamp to be inserted
     * @return true for success, false for failure
     */
    bool put(const char *key, const char *value, uint64_t timestamp);

    /**
     * get the value for a specific key
     *
     * @param key (in) key to be searched
     * @param value (out) value corresponding to the searched key
     * @param timestamp (out) timestamp of insertion
     * @return true if key was found, false if not
     */
    bool get(const char *key, const char **value, uint64_t *timestamp);

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
     * if we add this <key, value, timestamp> tuple, which will be the total
     * byte size of map?
     */
    uint64_t new_size(const char *key, const char *value, uint64_t timestamp);

    /**
     * clear all elements of map
     */
    void clear();

    struct cmp_str { // required, in order for the map to have its keys sorted
        bool operator()(char const *a, char const *b) {
            return strcmp(a, b) < 0;
        }
    };

    // map: key --> <value, timestamp>
    typedef pair<char *, uint64_t > KVTPair;
    typedef map<const char *, KVTPair, cmp_str> KVTMap;

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
    KVTMap::iterator start_iter(const char *key, bool key_incl);

    /**
     * return an iterator which points at the last element which equal or
     * less than 'key' if 'key_incl' is true, or strictly less than
     * 'key' if 'key_incl' is false.
     *
     * @param key key to be searched
     * @param key_incl whether the key should be inclusive or not
     * @return iterator at specific map element
     */
    KVTMap::iterator end_iter(const char *key, bool key_incl);

    KVTMap      m_map;
    uint64_t    m_size;            // size of map
    uint64_t    m_size_serialized; // size of map when written to disk
    uint64_t    m_keys;            // number of keys or tuples in map
};

#endif
