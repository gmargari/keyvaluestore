// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#include "./Global.h"
#include "./Map.h"

#include <stdlib.h>
#include <assert.h>
#include <utility>
#include <iostream>

#include "./Buffer.h"
#include "./Statistics.h"

using std::make_pair;
using std::cerr;
using std::endl;

/*============================================================================
 *                                    Map
 *============================================================================*/
Map::Map()
    : m_map(), m_size(0), m_size_serialized(0), m_keys(0) {
}

/*============================================================================
 *                                   ~Map
 *============================================================================*/
Map::~Map() {
    clear();
}

/*============================================================================
 *                                    put
 *============================================================================*/
bool Map::put(Slice Key, Slice Value, uint64_t timestamp) {
    const char *cpkey;
    char *cpvalue, *old_value, *p;
    uint64_t old_timestamp;
    uint32_t old_valuelen;
    KVTPair new_pair;
    KVTMap::iterator iter;
    const char *key = Key.data(), *value = Value.data();
    uint32_t keylen = Key.size(), valuelen = Value.size();


    assert(sanity_check());
    assert(key);
    assert(value);

    if (keylen > MAX_KEY_SIZE || valuelen > MAX_VALUE_SIZE) {
        cerr << "Error: key or value size greater than max size allowed, " <<
          MAX_KEY_SIZE << " and " << MAX_VALUE_SIZE << " respectively" << endl;
        exit(0);
        return false;
    }

    // if 'key' exists in map, delete corresponding value (it'll be replaced,
    // along with its timestamp)
    if ((iter = m_map.find(key)) != m_map.end()) {
        old_value = const_cast<char *>(iter->second.first.data());
        old_timestamp = iter->second.second;
        assert(timestamp > old_timestamp);

        old_valuelen = iter->second.first.size();
        m_size -= keylen + 1 + old_valuelen + 1 + sizeof(new_pair);
        m_size_serialized -= Buffer::serialize_len(keylen, old_valuelen,
                                                   timestamp);
        free(old_value);
        cpkey = key;
    } else {
        p = (char *)malloc(keylen + 1);
        memcpy(p, key, keylen + 1);
        cpkey = p;
        m_keys++;
    }

    p = (char *)malloc(valuelen + 1);
    memcpy(p, value, valuelen + 1);
    cpvalue = p;
    assert(cpkey);
    assert(cpvalue);

    new_pair.first = Slice(cpvalue, valuelen);
    new_pair.second = timestamp;
    m_size += keylen + 1 + sizeof(new_pair) + valuelen + 1;
    m_size_serialized += Buffer::serialize_len(keylen, valuelen, timestamp);
    m_map[cpkey] = new_pair;
    bytes_inc(&g_stats.bytes_inserted, keylen + valuelen);

    assert(sanity_check());

    return true;
}

/*============================================================================
 *                                    get
 *============================================================================*/
bool Map::get(Slice key, Slice *value, uint64_t *timestamp) {
    KVTMap::iterator iter;

    assert(sanity_check());
    assert(key.data() && value && timestamp);

    if ((iter = m_map.find(key.data())) != m_map.end()) {
        *value = iter->second.first;
        *timestamp = iter->second.second;
        return true;
    } else {
        *value = Slice();
        *timestamp = 0;
        return false;
    }
}

/*============================================================================
 *                                get_num_keys
 *============================================================================*/
uint64_t Map::get_num_keys() {
    return m_keys;
}

/*============================================================================
 *                                 get_size
 *============================================================================*/
uint64_t Map::get_size() {
    return m_size;
}

/*============================================================================
 *                            get_size_serialized
 *============================================================================*/
uint64_t Map::get_size_serialized() {
    return m_size_serialized;
}

/*============================================================================
 *                                  kv_size
 *============================================================================*/
uint32_t Map::kv_size(Slice key, Slice value, uint64_t timestamp) {
    return (key.size() + 1 + sizeof(KVTPair) + value.size() + 1);
}

/*============================================================================
 *                                   clear
 *============================================================================*/
void Map::clear() {
    Slice key, value;
    uint64_t timestamp;
#if DBGLVL > 0
    uint64_t dbg_mem_before, dbg_bytes_cleaned = 0;
#endif

    assert(sanity_check());
    assert((dbg_mem_before = get_size()) || 1);

    time_start(&(g_stats.free_time));

    for (KVTMap::iterator iter = m_map.begin(); iter != m_map.end(); ++iter) {
        key = iter->first;
        value = iter->second.first;
        timestamp = iter->second.second;
        assert(key.data());
        assert(value.data());
        assert((dbg_bytes_cleaned += kv_size(key, value, timestamp)));
        m_size -= key.size() + 1 + sizeof(KVTPair) + value.size() + 1;
        m_size_serialized -= Buffer::serialize_len(key.size(), value.size(),
                                                   timestamp);
        m_keys--;
        free(const_cast<char *>(key.data()));
        free(const_cast<char *>(value.data()));
    }

    m_map.clear();

    time_end(&(g_stats.free_time));

    assert(dbg_mem_before == dbg_bytes_cleaned);
    assert(get_size() == 0);
    assert(m_size == 0);
    assert(m_size_serialized == 0);
    assert(m_keys == 0);
    assert(sanity_check());
}

/*============================================================================
 *                                 start_iter
 *============================================================================*/
Map::KVTMap::iterator Map::start_iter(Slice key, bool key_incl) {
    KVTMap::iterator iter;

    assert(sanity_check());

    if (key.data()) {
        iter = m_map.lower_bound(key.data());
        if (key_incl == false && strcmp(iter->first.data(), key.data()) == 0) {
            ++iter;
        }
    } else {
        iter = m_map.begin();
    }

    return iter;
}

/*============================================================================
 *                                  end_iter
 *============================================================================*/
Map::KVTMap::iterator Map::end_iter(Slice key, bool key_incl) {
    KVTMap::iterator iter;

    assert(sanity_check());

    if (key.data()) {
        // upper_bound(x) returns an iterator pointing to the first element
        // whose key compares strictly greater than x
        iter = m_map.upper_bound(key.data());

        // if 'end_key' is not inclusive, set 'iter' one position back
        // and check if 'iter' has key equal to 'end_key'. if yes,
        // leave it there, else forward it one position.
        if (key_incl == false && iter != m_map.begin()) {
            --iter;
            if (strcmp(iter->first.data(), key.data()) != 0) {
                ++iter;
            }
        }
    } else {
        iter = m_map.end();
    }

    return iter;
}

/*============================================================================
 *                                sanity_check
 *============================================================================*/
int Map::sanity_check() {
#if DBGLVL > 1
    uint64_t map_size = 0, map_size_serialized = 0;
    const char *key, *value;
    uint64_t timestamp;
    uint32_t keylen, valuelen;

    for (KVTMap::iterator iter = m_map.begin(); iter != m_map.end(); ++iter) {
        key = iter->first.data();
        keylen = iter->first.size();
        value = iter->second.first.data();
        valuelen = iter->second.first.size();
        timestamp = iter->second.second;
        map_size += keylen + 1 + sizeof(KVTPair) + valuelen + 1;
        map_size_serialized += Buffer::serialize_len(keylen, valuelen,
                                                     timestamp);
        assert(keylen <= MAX_KEY_SIZE);
        assert(valuelen <= MAX_VALUE_SIZE);
        assert(timestamp != 0);
    }
    assert(m_size == map_size);
    assert(m_size_serialized == map_size_serialized);
#endif

    assert(m_keys == m_map.size());

    return 1;
}
