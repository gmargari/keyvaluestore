#ifndef KVTMAP_H
#define KVTMAP_H

#include <stdint.h>
#include <cstring>
#include <map>

using std::map;
using std::pair;

class KVTMap {

friend class KVTMapInputStream;

public:

    /**
     * constructor
     */
    KVTMap();

    /**
     * destructor
     */
    ~KVTMap();

    /**
     * clear all elements of map
     */
    void clear();

    /**
     * clear all elements of map with keys in the range ['start_key', 'end_key')
     * ('start_key' inclusive, 'end_key' exclusive)
     */
    void clear(const char *start_key, const char *end_key);

    /**
     * clear all elements of map with keys in the range [start_key, end_key].
     * 'start_key' and 'end_key' may or may not be included, depending on
     * 'start_incl' and 'end_incl'.
     */
    void clear(const char *start_key, const char *end_key, bool start_key_incl, bool end_key_incl);

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
     * insert a <key, value> pair into map. copies of the key and
     * value are created and inserted into map. a timestamp is automatically
     * assigned and the pair is store as a <key, value, timestamp> tuple in map.
     *
     * @param key key to be inserted
     * @param value value to be inserted
     * @return true for success, false for failure
     */
    bool put(const char *key, const char *value);

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
     * get the value for a specific <key, timestamp> pair
     *
     * @param key (in) key to be searched
     * @param timestamp (in) timestamp of insertion
     * @param value (out) value corresponding to the searched <key, timestamp>
     * @return true if <key, timestamp> was found, false if not
     */
    bool get(const char *key, uint64_t timestamp, const char **value);

    /**
     * number of <key, value, timestamp> tuples in map
     *
     * @return number of <key, value, timestamp> tuples in map
     */
    uint64_t num_keys();

    /**
     * total byte size of tuples stored in map
     *
     * @return total byte size of tuples stored in map
     */
    uint64_t size();

    /**
     * return current timestamp
     */
    uint64_t timestamp();

    struct cmp_str { // required, in order for the map to have its keys sorted
        bool operator()(char const *a, char const *b) {
            return strcmp(a, b) < 0;
        }
    };

    // map: key --> <value, timestamp>
    typedef pair<char *, uint64_t > kvtpair;
    typedef map<const char *, kvtpair, cmp_str> kvtmap;

protected:

    void sanity_check();

    /**
     * return an iterator pointing at the first element that is equal or
     * greater than 'key' if 'key_incl' is true, or strictly greater than
     * 'key' if 'key_incl' is false.
     *
     * @param key key to be searched
     * @param key_incl whether the key should be inclusive or not
     * @return iterator at specific map element
     */
    kvtmap::iterator start_iter(const char *key, bool key_incl);

    /**
     * return an iterator which points at the last element which equal or
     * less than 'key' if 'key_incl' is true, or strictly less than
     * 'key' if 'key_incl' is false.
     *
     * @param key key to be searched
     * @param key_incl whether the key should be inclusive or not
     * @return iterator at specific map element
     */
    kvtmap::iterator end_iter(const char *key, bool key_incl);

    kvtmap      m_map;
    uint64_t    m_size;
    uint64_t    m_keys;
};

#endif
