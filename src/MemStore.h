// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_MEMSTORE_H_
#define SRC_MEMSTORE_H_

#include <stdint.h>
#include <string.h>
#include <map>
#include <algorithm>
#include <vector>

#include "./Global.h"

using std::pair;
using std::vector;

class Map;
class MapInputStream;

class MemStore {
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
     * @param keylen size of key
     * @param value value to be inserted
     * @param valuelen size of value
     * @param timestamp timestamp to be inserted
     * @return true for success, false for failure
     */
    bool put(const char *key, uint32_t keylen, const char *value, uint32_t valuelen, uint64_t timestamp);

    /**
     * get the (copy of) value for a specific key.
     *
     * @param key (in) key to be searched
     * @param keylen (in) size of key
     * @param value (out) value corresponding to the searched key
     * @param valuelen (out) size of value
     * @param timestamp (out) timestamp of insertion
     * @return true if key was found, false if not
     */
    bool get(const char *key, uint32_t keylen, char **value, uint32_t *valuelen, uint64_t *timestamp);

    /**
     * return number of <key, value, timestamp> tuples in memstore
     */
    uint64_t get_num_keys();

    /**
     * return total byte size of tuples stored in memstore.
     */
    uint64_t get_size();

    /**
     * return total byte size of tuples when serialized to be stored on disk.
     */
    uint64_t get_size_when_serialized();

    /**
     * check if adding <key, value, timestamp> to memstore will reach memstore's
     * size limit
     */
    bool will_reach_size_limit(const char *key, uint32_t keylen, const char *value, uint32_t valuelen, uint64_t timestamp);

    /**
     * clear memstore
     */
    void clear();

    /**
     * create and return an input stream for memstore's map. NOTE: caller must delete
     * inputstream when done with it.
     */
    MapInputStream *new_map_inputstream();

    /**
     * create and return an input stream for memstore's map that contains key 'key'.
     * this function is used from rangemerge c.m., where we have multiple maps in
     * memstore, one per range. NOTE: caller must delete inputstream when done with it.
     */
    MapInputStream *new_map_inputstream(const char *key, uint32_t keylen);

    /**
     * when using rangemerge c.m. memstore consists of multiple maps, one for
     * each range. these functions add a new map for a newly created range,
     * find the map responsible for storing/retrieving a specific key, and
     * clear the map that contains a key
     */
    void add_map(const char *key, uint32_t keylen);
    int  get_num_maps();
    Map *get_map(const char *key, uint32_t keylen);
    Map *get_map(int i);
    void clear_map(const char *key, uint32_t keylen);
    void clear_map(int idx);

  protected:
    /**
     * used from add_map()/get_map()/clear_map() functions above
     * @return the index in vector 'm_map' of the map responsible for 'key'
     */
    int idx_of_map(const char *key, uint32_t keylen);

    /**
     * used from clear_map() functions above
     */
    void clear_map(Map *map);

    int sanity_check();

    typedef struct {
        char key[MAX_KVTSIZE];
        Map *map;
    } StrMapPair;

    static struct _pair_compare {
        inline bool operator() (const StrMapPair & left, const StrMapPair right) {
            return strcmp(left.key, right.key);
        }
        inline bool operator() (const StrMapPair & left, char * const & right) {
            return strcmp(left.key, right);
        }
        inline bool operator() (char * const & left, const StrMapPair & right) {
            return strcmp(left, right.key);
        }
    } pair_compare;

    vector < StrMapPair > m_map;
    uint64_t  m_maxsize;
    uint64_t  m_num_keys;
    uint64_t  m_size;
    uint64_t  m_size_when_serialized;
};

#endif  // SRC_MEMSTORE_H_
