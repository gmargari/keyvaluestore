#ifndef KVMAP_H
#define KVMAP_H

#include "Global.h"

#include <stdint.h>
#include <cstring>
#include <map>

class KVMap {

friend class KVMapInputStream;

public:
    
    /**
     * constructor
     */
    KVMap();
    
    /**
     * destructor
     */
    ~KVMap();

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
     * insert a <key, value> pair into map. copies of the key and value are
     * created and inserted into the map.
     *
     * @param key key to be inserted
     * @param value value to inserted
     * @return true for success, false for failure
     */
    bool put(const char *key, const char *value);

    /**
     * get the value for a specific key
     * 
     * @param key key to be searched
     * @return pointer to value of specified key, NULL if key does not exist in
     * map
     */
    const char *get(const char *key);

    /**
     * number of <key, value> pairs in map
     *
     * @return number of <key, value> pairs in map
     */
    uint64_t num_keys();

    /**
     * size of <key, value> pairs in map (in bytes)
     *
     * @return size of <key, value> pairs in map (in bytes)
     */
    uint64_t size();

    struct cmp_str { // required, in order for the map to have its keys sorted
        bool operator()(char const *a, char const *b) {
            return strcmp(a, b) < 0;
        }
    };

    typedef std::map<const char *, char *, cmp_str> kvmap;
    
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
    kvmap::iterator start_iter(const char *key, bool key_incl);

    /**
     * return an iterator which points at the last element which equal or
     * less than 'key' if 'key_incl' is true, or strictly less than
     * 'key' if 'key_incl' is false.
     *
     * @param key key to be searched
     * @param key_incl whether the key should be inclusive or not
     * @return iterator at specific map element
     */
    kvmap::iterator end_iter(const char *key, bool key_incl);

    kvmap       m_map;
    uint64_t    m_size;
    uint64_t    m_keys;
};

#endif
