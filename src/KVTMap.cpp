#include "Global.h"
#include "KVTMap.h"

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <sys/time.h>

using namespace std;

/*========================================================================
 *                                 KVTMap
 *========================================================================*/
KVTMap::KVTMap()
{
    m_size = 0;
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
 *                                 clear
 *=======================================================================*/
void KVTMap::clear()
{
    clear(NULL, NULL, true, true);
    assert(m_size == 0);
    assert(m_keys == 0);
}

/*=======================================================================*
 *                                 clear
 *=======================================================================*/
void KVTMap::clear(const char *start_key, const char *end_key)
{
    clear(start_key, end_key, true, false);
}

/*=======================================================================*
 *                                 clear
 *=======================================================================*/
void KVTMap::clear(const char *start_key, const char *end_key, bool start_key_incl, bool end_key_incl)
{
    kvtmap::iterator iter, s_iter, e_iter;
    sanity_check();

    s_iter = start_iter(start_key, start_key_incl);
    e_iter = end_iter(end_key, end_key_incl);
    for(iter = s_iter; iter != e_iter; iter++) {
        assert(iter->first);
        assert(iter->second);
        m_size -= strlen(iter->first) + strlen(iter->second) + 2;
        m_keys--;
        free(const_cast<char*>(iter->first));
        free(iter->second);
    }
    m_map.erase(s_iter, e_iter);

    sanity_check();
}

/*=======================================================================*
 *                                  put
 *=======================================================================*/
bool KVTMap::put(const char *key, const char *value, uint64_t timestamp)
{
    char *cpvalue;
    const char *cpkey;

    sanity_check();
    assert(key);
    assert(value);

    if (strlen(key) + 1 > MAX_KVTSIZE || strlen(value) + 1 > MAX_KVTSIZE) {
        printf("Error: key or value size greater than max size allowed (%ld)\n", MAX_KVTSIZE);
        assert(0);
        return false;
    }

    // if 'key' exists in map, delete corresponding value (it'll be replaced)
    kvtmap::iterator f = m_map.find(key);
    if (f != m_map.end()) {
        m_size -= strlen(f->second) + 1;
        free(const_cast<char*>(f->second));
        cpkey = key;
    } else {
        cpkey = strdup(key);
        m_size += strlen(cpkey) + 1;
        m_keys++;
    }

    cpvalue = strdup(value);
    assert(cpkey);
    assert(cpvalue);

    m_size += strlen(cpvalue) + 1;
    m_map[cpkey] = cpvalue;

    sanity_check();

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
    sanity_check();
    kvtmap::iterator f = m_map.find(key);
    if (f != m_map.end()) {
        *value = f->second;
        *timestamp = 666;
        return true;
    } else {
        *value = NULL;
        *timestamp = 0;
        return false;
    }
}

// TODO: implement
// /*=======================================================================*
//  *                                  get
//  *=======================================================================*/
// bool KVTMap::get(const char *key, uint64_t timestamp, const char **value)
// {
//
// }

/*=======================================================================*
 *                                 size
 *=======================================================================*/
uint64_t KVTMap::size()
{
    sanity_check();
    return m_size;
}


/*=======================================================================*
 *                               num_keys
 *=======================================================================*/
uint64_t KVTMap::num_keys()
{
    sanity_check();
    return m_keys;
}

/*=======================================================================*
 *                               num_keys
 *=======================================================================*/
KVTMap::kvtmap::iterator KVTMap::start_iter(const char *key, bool key_incl)
{
    kvtmap::iterator iter;

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
 *                               num_keys
 *=======================================================================*/
KVTMap::kvtmap::iterator KVTMap::end_iter(const char *key, bool key_incl)
{
    kvtmap::iterator iter;

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
 *                              get_timestamp
 *=======================================================================*/
uint64_t KVTMap::timestamp()
{
//     struct timeval tv;
//
//     gettimeofday(&tv, NULL);
//     return (uint64_t)(tv.tv_sec*1000000 + tv.tv_usec);
    static int i = 0;
    return i++;
}

/*=======================================================================*
 *                              sanity_check
 *=======================================================================*/
void KVTMap::sanity_check()
{
    uint64_t map_size = 0;

    return_if_dbglvl_lt_2();

    for(kvtmap::iterator iter = m_map.begin(); iter != m_map.end(); iter++) {
        map_size += strlen(iter->first) + strlen(iter->second) + 2;
        assert(strlen(iter->first) + 1 <= MAX_KVTSIZE);
        assert(strlen(iter->second) + 1 <= MAX_KVTSIZE);
    }
    assert(m_size == map_size);
    assert(m_keys == m_map.size());
}
