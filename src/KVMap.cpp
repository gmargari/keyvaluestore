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
    sanity_check();
    
    for(kvmap::iterator iter = m_map.begin(); iter != m_map.end(); iter++) {
        assert(iter->first);
        assert(iter->second);
        free(const_cast<char*>(iter->first));
        free(iter->second);
    }
    m_map.clear();
    m_size = 0;
    m_keys = 0;
    
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
char *KVMap::get(char *key)
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
 *                                 size
 *=======================================================================*/
uint64_t KVMap::num_keys()
{
    sanity_check();
    return m_keys;
}

/*=======================================================================*
 *                              sanity_check
 *=======================================================================*/
void KVMap::sanity_check()
{
    uint64_t map_size = 0;

#if DBGLVL < 2
    return;
#endif
    
    for(kvmap::iterator iter = m_map.begin(); iter != m_map.end(); iter++) {
        map_size += strlen(iter->first) + strlen(iter->second) + 2;
        assert(strlen(iter->first) + 1 <= MAX_KVSIZE);
        assert(strlen(iter->second) + 1 <= MAX_KVSIZE);
    }
    assert(m_size == map_size);
    assert(m_keys == m_map.size());
}
