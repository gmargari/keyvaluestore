#include "KVMap.h"

#include <cassert>
#include <cstdlib>

using namespace std;

/*========================================================================
 *                                 KVMap
 *========================================================================*/
KVMap::KVMap()
{
    clear();
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
    for(kvmap::iterator iter = m_map.begin(); iter != m_map.end(); iter++) {
        if ((*iter).first) {
            free(const_cast<char*>(iter->first));
        }
        if ((*iter).second) {
            free(iter->second);
        }
    }
    m_map.clear();
}

/*=======================================================================*
 *                                  put
 *=======================================================================*/
bool KVMap::put(const char *key, const char *value)
{
    char *cpvalue;
    const char *cpkey;
    
    assert(key);
    assert(value);
    // if 'key' exists in map, delete corresponding value (it'll bereplaced)
    kvmap::iterator f = m_map.find(key);
    if (f != m_map.end()) {
        free(const_cast<char*>(f->second));
        cpkey = key;
    } else {
        cpkey = strdup(key);
    }
    cpvalue = strdup(value);
    assert(cpkey);
    assert(cpvalue);
    m_map[cpkey] = cpvalue;

    return true;
}

/*=======================================================================*
 *                                  get
 *=======================================================================*/
char *KVMap::get(char *key)
{
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
    // total size of keys and values in map (in bytes)
    return 0;
}


/*=======================================================================*
 *                                 size
 *=======================================================================*/
uint64_t KVMap::num_keys()
{
    return m_map.size();
}

// // TEST
// KVMap::print_kvs()
// {
//     for(kvmap::iterator iter = m_map.begin(); iter != m_map.end(); iter++) {
//         printf("\t[%p -> %p] = ['%s' -> '%s']\n", (*iter).first, (*iter).second, (*iter).first, (*iter).second);
//     }
//     printf("\n");
// }
