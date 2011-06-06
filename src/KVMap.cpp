#include "Global.h"
#include "KVMap.h"

#include <cassert>
#include <cstdlib>

using namespace std;

/*========================================================================
 *                                 KVMap
 *========================================================================*/
KVMap::KVMap()
{
    m_size = 0;
    m_keys = 0;
}

/*========================================================================
 *                                ~KVMap
 *========================================================================*/
KVMap::~KVMap()
{
    clear();
}

/*=======================================================================*
 *                                 clear
 *=======================================================================*/
void KVMap::clear()
{
    clear(NULL, NULL, true, true);
    assert(m_size == 0);
    assert(m_keys == 0);
}

/*=======================================================================*
 *                                 clear
 *=======================================================================*/
void KVMap::clear(const char *start_key, const char *end_key)
{
    clear(start_key, end_key, true, false);
}

/*=======================================================================*
 *                                 clear
 *=======================================================================*/
void KVMap::clear(const char *start_key, const char *end_key, bool start_key_incl, bool end_key_incl)
{
    kvmap::iterator iter, s_iter, e_iter;
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

#include <stdio.h>

/*=======================================================================*
 *                                  put
 *=======================================================================*/
bool KVMap::put(const char *key, const char *value)
{
    char *cpvalue;
    const char *cpkey;

    sanity_check();
    assert(key);
    assert(value);

    if (strlen(key) + 1 > MAX_KVSIZE || strlen(value) + 1> MAX_KVSIZE) {
        printf("Error: key or value size greater than max size allowed (%ld)\n", MAX_KVSIZE);
        assert(0);
        return false;
    }

    // if 'key' exists in map, delete corresponding value (it'll be replaced)
    kvmap::iterator f = m_map.find(key);
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
 *                                  get
 *=======================================================================*/
const char *KVMap::get(const char *key)
{
    sanity_check();
    kvmap::iterator f = m_map.find(key);
    if (f != m_map.end()) {
        return f->second;
    } else {
        return NULL;
    }
}

/*=======================================================================*
 *                                 size
 *=======================================================================*/
uint64_t KVMap::size()
{
    sanity_check();
    return m_size;
}


/*=======================================================================*
 *                               num_keys
 *=======================================================================*/
uint64_t KVMap::num_keys()
{
    sanity_check();
    return m_keys;
}

/*=======================================================================*
 *                               num_keys
 *=======================================================================*/
KVMap::kvmap::iterator KVMap::start_iter(const char *key, bool key_incl)
{
    kvmap::iterator iter;

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
KVMap::kvmap::iterator KVMap::end_iter(const char *key, bool key_incl)
{
    kvmap::iterator iter;

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
void KVMap::sanity_check()
{
    uint64_t map_size = 0;

    return_if_dbglvl_lt_2();

    for(kvmap::iterator iter = m_map.begin(); iter != m_map.end(); iter++) {
        map_size += strlen(iter->first) + strlen(iter->second) + 2;
        assert(strlen(iter->first) + 1 <= MAX_KVSIZE);
        assert(strlen(iter->second) + 1 <= MAX_KVSIZE);
    }
    assert(m_size == map_size);
    assert(m_keys == m_map.size());
}
