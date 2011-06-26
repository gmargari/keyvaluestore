#include "Global.h"
#include "KVTMap.h"

#include "KVTSerialization.h"

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <sys/time.h>

using std::make_pair;

/*========================================================================
 *                                 KVTMap
 *========================================================================*/
KVTMap::KVTMap()
{
    m_size = 0;
    m_size_serialized = 0;
    m_keys = 0;
}

/*========================================================================
 *                                ~KVTMap
 *========================================================================*/
KVTMap::~KVTMap()
{
    clear();
}

/*=======================================================================*
 *                                  put
 *=======================================================================*/
bool KVTMap::put(const char *key, const char *value, uint64_t timestamp)
{
    const char *cpkey, *f_key;
    char *cpvalue, *f_value;
    uint64_t f_timestamp;
    size_t strlen_key, strlen_value;
    kvtpair new_pair;
    kvtmap::iterator f;

    assert(sanity_check());
    assert(key);
    assert(value);

    strlen_key = strlen(key);
    strlen_value = strlen(value);
    if (strlen_key + 1 > MAX_KVTSIZE || strlen_value + 1 > MAX_KVTSIZE) {
        printf("Error: key or value size greater than max size allowed (%ld)\n", MAX_KVTSIZE);
        assert(0);
        return false;
    }

    // if 'key' exists in map, delete corresponding value (it'll be replaced,
    // along with its timestamp)
    if ((f = m_map.find(key)) != m_map.end()) {
        f_key = const_cast<char *>(f->first);
        f_value = const_cast<char *>(f->second.first);
        f_timestamp = f->second.second;
        assert(timestamp > f_timestamp);

        m_size -= strlen_key + 1 + strlen(f_value) + 1 + sizeof(new_pair);
        m_size_serialized -= serialize_len(strlen_key, strlen(f_value), timestamp);
        free(f_value);
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
    m_size += strlen_key + 1 + sizeof(new_pair) + strlen_value + 1;
    m_size_serialized += serialize_len(strlen_key, strlen_value, timestamp);
    m_map[cpkey] = new_pair;

    assert(sanity_check());

    return true;
}

/*=======================================================================*
 *                                  put
 *=======================================================================*/
bool KVTMap::put(const char *key, const char *value)
{
    return put(key, value, timestamp());
}

/*=======================================================================*
 *                                  get
 *=======================================================================*/
bool KVTMap::get(const char *key, const char **value, uint64_t *timestamp)
{
    kvtmap::iterator f;

    assert(sanity_check());
    assert(key && value && timestamp);

    if ((f = m_map.find(key)) != m_map.end()) {
        *value = f->second.first;
        *timestamp = f->second.second;
        return true;
    } else {
        *value = NULL;
        *timestamp = 0;
        return false;
    }
}

/*=======================================================================*
 *                                  get
 *=======================================================================*/
bool KVTMap::get(const char *key, uint64_t timestamp, const char **value)
{
    uint64_t ts;

    if (get(key, value, &ts) && ts == timestamp) {
        return true;
    } else {
        *value = NULL;
        return false;
    }
}

/*=======================================================================*
 *                             get_num_keys
 *=======================================================================*/
uint64_t KVTMap::get_num_keys()
{
    return m_keys;
}

/*=======================================================================*
 *                               get_size
 *=======================================================================*/
uint64_t KVTMap::get_size()
{
    return get_sizes().first;
}

/*=======================================================================*
 *                               get_size
 *=======================================================================*/
uint64_t KVTMap::get_size(const char *start_key, const char *end_key, bool start_key_incl, bool end_key_incl)
{
    return get_sizes(start_key, end_key, start_key_incl, end_key_incl).first;
}

/*=======================================================================*
 *                         get_size_when_serialized
 *=======================================================================*/
uint64_t KVTMap::get_size_when_serialized()
{
    return get_sizes().second;
}

/*=======================================================================*
 *                         get_size_when_serialized
 *=======================================================================*/
uint64_t KVTMap::get_size_when_serialized(const char *start_key, const char *end_key, bool start_key_incl, bool end_key_incl)
{
    return get_sizes(start_key, end_key, start_key_incl, end_key_incl).second;
}

/*=======================================================================*
 *                               get_sizes
 *=======================================================================*/
pair<uint64_t, uint64_t> KVTMap::get_sizes()
{
    return make_pair(m_size, m_size_serialized);
}

/*=======================================================================*
 *                               get_sizes
 *=======================================================================*/
pair<uint64_t, uint64_t> KVTMap::get_sizes(const char *start_key, const char *end_key, bool start_key_incl, bool end_key_incl)
{
    kvtmap::iterator iter, s_iter, e_iter;
    char *key, *value;
    uint64_t timestamp, memsize, memsize_serialized;
    pair<uint64_t, uint64_t> ret;

    assert(sanity_check());

    s_iter = start_iter(start_key, start_key_incl);
    e_iter = end_iter(end_key, end_key_incl);
    memsize = 0;
    memsize_serialized = 0;
    for(iter = s_iter; iter != e_iter; iter++) {
        key = const_cast<char *>(iter->first);
        value = const_cast<char *>(iter->second.first);
        timestamp = iter->second.second;
        assert(key);
        assert(value);
        memsize += strlen(key) + 1 + sizeof(kvtpair) + strlen(value) + 1;
        memsize_serialized += serialize_len(strlen(key), strlen(value), timestamp);
    }

    assert(sanity_check());

    ret.first = memsize;
    ret.second = memsize_serialized;
    return ret;
}

/*=======================================================================*
 *                                 new_size
 *=======================================================================*/
uint64_t KVTMap::new_size(const char *key, const char *value, uint64_t timestamp)
{
    return get_size() + strlen(key) + 1 + sizeof(kvtpair) + strlen(value) + 1;
}

/*=======================================================================*
 *                                 clear
 *=======================================================================*/
void KVTMap::clear()
{
    clear(NULL, NULL, true, true);
    assert(m_size == 0);
    assert(m_size_serialized == 0);
    assert(m_keys == 0);
}

/*=======================================================================*
 *                                 clear
 *=======================================================================*/
void KVTMap::clear(const char *start_key, const char *end_key, bool start_key_incl, bool end_key_incl)
{
    kvtmap::iterator iter, s_iter, e_iter;
    char *key, *value;
    uint64_t timestamp;
    uint64_t dbg_mem_before, dbg_to_clean, dbg_bytes_cleaned = 0; // for debugging only

    assert(sanity_check());
    assert((dbg_to_clean = get_size(start_key, end_key, start_key_incl, end_key_incl)) || 1);
    assert((dbg_mem_before = get_size()) || 1);

    s_iter = start_iter(start_key, start_key_incl);
    e_iter = end_iter(end_key, end_key_incl);
    for(iter = s_iter; iter != e_iter; iter++) {
        key = const_cast<char *>(iter->first);
        value = const_cast<char *>(iter->second.first);
        timestamp = iter->second.second;
        assert(key);
        assert(value);
        assert((dbg_bytes_cleaned += strlen(key) + 1 + sizeof(kvtpair) + strlen(value) + 1));
        m_size -= strlen(key) + 1 + sizeof(kvtpair) + strlen(value) + 1;
        m_size_serialized -= serialize_len(strlen(key), strlen(value), timestamp);
        m_keys--;
        free(key);
        free(value);
    }
    m_map.erase(s_iter, e_iter);

    assert(dbg_to_clean == dbg_bytes_cleaned);
    assert(get_size(start_key, end_key, start_key_incl, end_key_incl) == 0);
    assert(dbg_mem_before - get_size() == dbg_to_clean);
    assert(sanity_check());
}

/*=======================================================================*
 *                              get_timestamp
 *=======================================================================*/
uint64_t KVTMap::timestamp()
{
//     struct timeval tv;
//
//     gettimeofday(&tv, NULL);
//     return (uint64_t)(tv.tv_sec*1000000 + tv.tv_usec);
    static int i = 1;
    return i++;
}

/*=======================================================================*
 *                               start_iter
 *=======================================================================*/
KVTMap::kvtmap::iterator KVTMap::start_iter(const char *key, bool key_incl)
{
    kvtmap::iterator iter;

    assert(sanity_check());

    if (key) {
        iter = m_map.lower_bound(key);
        if (key_incl == false && strcmp(iter->first, key) == 0) {
            iter++;
        }
    } else {
        iter = m_map.begin();
    }

    return iter;
}

/*=======================================================================*
 *                               end_iter
 *=======================================================================*/
KVTMap::kvtmap::iterator KVTMap::end_iter(const char *key, bool key_incl)
{
    kvtmap::iterator iter;

    assert(sanity_check());

    if (key) {
        // upper_bound(x) returns an iterator pointing to the first element
        // whose key compares strictly greater than x
        iter = m_map.upper_bound(key);

        // if 'end_key' is not inclusive, set 'iter' one position back
        // and check if 'iter' has key equal to 'end_key'. if yes,
        // leave it there, else forward it one position.
        if (key_incl == false && iter != m_map.begin()) {
            iter--;
            if (strcmp(iter->first, key) != 0) {
                iter++;
            }
        }
    } else {
        iter = m_map.end();
    }

    return iter;
}

/*=======================================================================*
 *                              sanity_check
 *=======================================================================*/
int KVTMap::sanity_check()
{
#if DBGLVL > 1
    uint64_t map_size = 0, map_size_serialized = 0;
    const char *key, *value;
    uint64_t timestamp;

    for(kvtmap::iterator iter = m_map.begin(); iter != m_map.end(); iter++) {
        key = iter->first;
        value = iter->second.first;
        timestamp = iter->second.second;

        map_size += strlen(key) + 1 + sizeof(kvtpair) + strlen(value) + 1;
        map_size_serialized += serialize_len(strlen(key), strlen(value), timestamp);
        assert(strlen(key) + 1 <= MAX_KVTSIZE);
        assert(strlen(value) + 1 <= MAX_KVTSIZE);
        assert(timestamp != 0);
    }
    assert(m_size == map_size);
    assert(m_size_serialized == map_size_serialized);
#endif

    assert(m_keys == m_map.size());

    return 1;
}
