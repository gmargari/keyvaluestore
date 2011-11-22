#include "Global.h"
#include "Map.h"

#include "Buffer.h"
#include "Statistics.h"

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <sys/time.h>

using std::make_pair;

/*============================================================================
 *                                    Map
 *============================================================================*/
Map::Map()
    : m_map(), m_size(0), m_size_serialized(0), m_keys(0)
{

}

/*============================================================================
 *                                   ~Map
 *============================================================================*/
Map::~Map()
{
    clear();
}

/*============================================================================
 *                                    put
 *============================================================================*/
bool Map::put(const char *key, const char *value, uint64_t timestamp)
{
    const char *cpkey;
    char *cpvalue, *old_value;
    uint64_t old_timestamp;
    size_t keylen, valuelen, old_valuelen;
    KVTPair new_pair;
    KVTMap::iterator iter;

    assert(sanity_check());
    assert(key);
    assert(value);

    keylen = strlen(key);
    valuelen = strlen(value);
    if (keylen + 1 > MAX_KVTSIZE || valuelen + 1 > MAX_KVTSIZE) {
        printf("Error: key or value size greater than max size allowed (%ld)\n", MAX_KVTSIZE);
        assert(0);
        return false;
    }

    // if 'key' exists in map, delete corresponding value (it'll be replaced,
    // along with its timestamp)
    if ((iter = m_map.find(key)) != m_map.end()) {
        old_value = const_cast<char *>(iter->second.first);
        old_timestamp = iter->second.second;
        assert(timestamp > old_timestamp);

        old_valuelen = strlen(old_value);
        m_size -= keylen + 1 + old_valuelen + 1 + sizeof(new_pair);
        m_size_serialized -= Buffer::serialize_len(keylen, old_valuelen, timestamp);
        free(old_value);
        cpkey = key;
    } else {
        cpkey = strdup(key);
        m_keys++;
    }

    cpvalue = strdup(value);
    assert(cpkey);
    assert(cpvalue);

    new_pair.first = cpvalue;
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
bool Map::get(const char *key, const char **value, uint64_t *timestamp)
{
    KVTMap::iterator iter;

    assert(sanity_check());
    assert(key && value && timestamp);

    if ((iter = m_map.find(key)) != m_map.end()) {
        *value = iter->second.first;
        *timestamp = iter->second.second;
        return true;
    } else {
        *value = NULL;
        *timestamp = 0;
        return false;
    }
}

/*============================================================================
 *                                get_num_keys
 *============================================================================*/
uint64_t Map::get_num_keys()
{
    return m_keys;
}

/*============================================================================
 *                                 get_size
 *============================================================================*/
uint64_t Map::get_size()
{
    return get_sizes().first;
}

/*============================================================================
 *                                 get_size
 *============================================================================*/
uint64_t Map::get_size(const char *start_key, const char *end_key, bool start_key_incl, bool end_key_incl)
{
    return get_sizes(start_key, end_key, start_key_incl, end_key_incl).first;
}

/*============================================================================
 *                          get_size_when_serialized
 *============================================================================*/
uint64_t Map::get_size_when_serialized()
{
    return get_sizes().second;
}

/*============================================================================
 *                          get_size_when_serialized
 *============================================================================*/
uint64_t Map::get_size_when_serialized(const char *start_key, const char *end_key, bool start_key_incl, bool end_key_incl)
{
    return get_sizes(start_key, end_key, start_key_incl, end_key_incl).second;
}

/*============================================================================
 *                                get_sizes
 *============================================================================*/
pair<uint64_t, uint64_t> Map::get_sizes()
{
    return make_pair(m_size, m_size_serialized);
}

/*============================================================================
 *                                get_sizes
 *============================================================================*/
pair<uint64_t, uint64_t> Map::get_sizes(const char *start_key, const char *end_key, bool start_key_incl, bool end_key_incl)
{
    KVTMap::iterator iter, s_iter, e_iter;
    char *key, *value;
    uint64_t timestamp, memsize, memsize_serialized;
    pair<uint64_t, uint64_t> ret;
    size_t keylen, valuelen;

    assert(sanity_check());

    s_iter = start_iter(start_key, start_key_incl);
    e_iter = end_iter(end_key, end_key_incl);
    memsize = 0;
    memsize_serialized = 0;
    for(iter = s_iter; iter != e_iter; ++iter) {
        key = const_cast<char *>(iter->first);
        value = const_cast<char *>(iter->second.first);
        timestamp = iter->second.second;
        assert(key);
        assert(value);
        keylen = strlen(key);
        valuelen = strlen(value);
        memsize += keylen + 1 + sizeof(KVTPair) + valuelen + 1;
        memsize_serialized += Buffer::serialize_len(keylen, valuelen, timestamp);
    }

    assert(sanity_check());

    ret.first = memsize;
    ret.second = memsize_serialized;
    return ret;
}

/*============================================================================
 *                                  new_size
 *============================================================================*/
uint64_t Map::new_size(const char *key, const char *value, uint64_t timestamp)
{
    return get_size() + strlen(key) + 1 + sizeof(KVTPair) + strlen(value) + 1;
}

/*============================================================================
 *                                   clear
 *============================================================================*/
void Map::clear()
{
    clear(NULL, NULL, true, true);

    assert(m_size == 0);
    assert(m_size_serialized == 0);
    assert(m_keys == 0);
}

/*============================================================================
 *                                   clear
 *============================================================================*/
void Map::clear(const char *start_key, const char *end_key, bool start_key_incl, bool end_key_incl)
{
    KVTMap::iterator iter, s_iter, e_iter;
    char *key, *value;
    uint64_t timestamp;
    size_t keylen, valuelen;
#if DBGLVL > 0
    uint64_t dbg_mem_before, dbg_to_clean, dbg_bytes_cleaned = 0;
#endif

    assert(sanity_check());
    assert((dbg_to_clean = get_size(start_key, end_key, start_key_incl, end_key_incl)) || 1);
    assert((dbg_mem_before = get_size()) || 1);

    time_start(&(g_stats.free_time));

    s_iter = start_iter(start_key, start_key_incl);
    e_iter = end_iter(end_key, end_key_incl);
    for(iter = s_iter; iter != e_iter; ++iter) {
        key = const_cast<char *>(iter->first);
        value = const_cast<char *>(iter->second.first);
        timestamp = iter->second.second;
        keylen = strlen(key);
        valuelen = strlen(value);
        assert(key);
        assert(value);
        assert((dbg_bytes_cleaned += keylen + 1 + sizeof(KVTPair) + valuelen + 1));
        m_size -= keylen + 1 + sizeof(KVTPair) + valuelen + 1;
        m_size_serialized -= Buffer::serialize_len(keylen, valuelen, timestamp);
        m_keys--;
        free(key);
        free(value);
    }

    if (start_key != NULL || end_key != NULL) {
        m_map.erase(s_iter, e_iter);
    } else {
        m_map.clear();
    }

    time_end(&(g_stats.free_time));

    assert(dbg_to_clean == dbg_bytes_cleaned);
    assert(get_size(start_key, end_key, start_key_incl, end_key_incl) == 0);
    assert(dbg_mem_before - get_size() == dbg_to_clean);
    assert(sanity_check());
}

/*============================================================================
 *                                 start_iter
 *============================================================================*/
Map::KVTMap::iterator Map::start_iter(const char *key, bool key_incl)
{
    KVTMap::iterator iter;

    assert(sanity_check());

    if (key) {
        iter = m_map.lower_bound(key);
        if (key_incl == false && strcmp(iter->first, key) == 0) {
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
Map::KVTMap::iterator Map::end_iter(const char *key, bool key_incl)
{
    KVTMap::iterator iter;

    assert(sanity_check());

    if (key) {
        // upper_bound(x) returns an iterator pointing to the first element
        // whose key compares strictly greater than x
        iter = m_map.upper_bound(key);

        // if 'end_key' is not inclusive, set 'iter' one position back
        // and check if 'iter' has key equal to 'end_key'. if yes,
        // leave it there, else forward it one position.
        if (key_incl == false && iter != m_map.begin()) {
            --iter;
            if (strcmp(iter->first, key) != 0) {
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
int Map::sanity_check()
{
#if DBGLVL > 1
    uint64_t map_size = 0, map_size_serialized = 0;
    const char *key, *value;
    uint64_t timestamp;
    size_t keylen, valuelen;

    for(KVTMap::iterator iter = m_map.begin(); iter != m_map.end(); ++iter) {
        key = iter->first;
        value = iter->second.first;
        timestamp = iter->second.second;
        keylen = strlen(key);
        valuelen = strlen(value);
        map_size += keylen + 1 + sizeof(KVTPair) + valuelen + 1;
        map_size_serialized += Buffer::serialize_len(keylen, valuelen, timestamp);
        assert(keylen + 1 <= MAX_KVTSIZE);
        assert(valuelen + 1 <= MAX_KVTSIZE);
        assert(timestamp != 0);
    }
    assert(m_size == map_size);
    assert(m_size_serialized == map_size_serialized);
#endif

    assert(m_keys == m_map.size());

    return 1;
}
