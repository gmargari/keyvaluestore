// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_MEMSTORE_H_
#define SRC_MEMSTORE_H_

#include <stdint.h>
#include <string.h>
#include <map>
#include <algorithm>
#include <vector>

#include "./Global.h"
#include "./Slice.h"

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
     * @param value value to be inserted
     * @param timestamp timestamp to be inserted
     * @return true for success, false for failure
     */
    bool put(Slice key, Slice value, uint64_t timestamp);

    /**
     * get the (copy of) value for a specific key.
     *
     * @param key (in) key to be searched
     * @param value (out) value corresponding to the searched key
     * @param timestamp (out) timestamp of insertion
     * @return true if key was found, false if not
     */
    bool get(Slice key, Slice *value, uint64_t *timestamp);

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
     * check if adding <key, value, timestamp> to memstore will cause memstore
     * to pass its size limit
     */
    bool will_fill(Slice key, Slice value, uint64_t timestamp);

    /**
     * clear memstore's map
     */
    void clear_map();

    /**
     * create and return an input stream for memstore's map. NOTE: caller must
     * delete inputstream when done with it.
     */
    MapInputStream *new_map_inputstream();

    /**
     * when using rangemerge c.m., memstore consists of multiple maps, one for
     * each range. functions below add a new map for a newly created range,
     * find the map responsible for storing/retrieving a specific key, clear
     * the map that contains a key, and create and return an input stream for
     * the map containing a specific key
     */
    void add_map(Slice key);
    Map *get_map(Slice key);
    void clear_map(Slice key);
    MapInputStream *new_map_inputstream(Slice key);

  private:
    /**
     * used from add_map()/get_map()/clear_map() functions above
     * @return the index in vector 'm_map' of the map responsible for 'key'
     */
    int idx_of_map(Slice key);

    /**
     * used from clear_map() functions above
     */
    void clear_map(Map *map);

    int sanity_check();

    typedef struct {
        char key[MAX_KEY_SIZE + 1];
        Map *map;
    } StrMapPair;

    static struct _pair_compare {
        inline bool operator() (const StrMapPair left, const StrMapPair right) {
            return strcmp(left.key, right.key);
        }
        inline bool operator() (const StrMapPair left, char * const right) {
            return strcmp(left.key, right);
        }
        inline bool operator() (char * const left, const StrMapPair right) {
            return strcmp(left, right.key);
        }
    } pair_compare;

    vector<StrMapPair> m_map;
    uint64_t m_maxsize;
    uint64_t m_num_keys;
    uint64_t m_size;
    uint64_t m_size_when_serialized;
};

#endif  // SRC_MEMSTORE_H_
