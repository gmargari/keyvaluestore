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
     * clear the map
     */
    void clear();

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
     * map (return value is not copied, caller must copy it)
     */
    char *get(char *key);

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
    
    kvmap m_map;
};

#endif
