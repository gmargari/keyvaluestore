#include "Global.h"
#include "MemStore.h"

#include "Map.h"
#include "MapInputStream.h"

#include <assert.h>

using std::make_pair;

/*============================================================================
 *                                 MemStore
 *============================================================================*/
MemStore::MemStore()
    : m_map(), m_maxsize(DEFAULT_MEMSTORE_SIZE)
{
    add_map((char *)"");
}

/*============================================================================
 *                                ~MemStore
 *============================================================================*/
MemStore::~MemStore()
{
    for (int i = 0; i < (int)m_map.size(); i++) {
        delete m_map[i].map;
    }
}

/*============================================================================
 *                               set_maxsize
 *============================================================================*/
void MemStore::set_maxsize(uint64_t maxsize)
{
    m_maxsize = maxsize;
}

/*============================================================================
 *                               get_maxsize
 *============================================================================*/
uint64_t MemStore::get_maxsize()
{
    return m_maxsize;
}

/*============================================================================
 *                                   put
 *============================================================================*/
bool MemStore::put(const char *key, const char *value, uint64_t timestamp)
{
    return get_map(key)->put((char *)key, value, timestamp);
}

/*============================================================================
 *                                   get
 *============================================================================*/
bool MemStore::get(const char *key, char **value, uint64_t *timestamp)
{
    const char *constvalue;

    if (get_map(key)->get(key, &constvalue, timestamp)) {
        *value = strdup(constvalue); // copy value
        return true;
    } else {
        return false;
    }
}

/*============================================================================
 *                              get_num_keys
 *============================================================================*/
uint64_t MemStore::get_num_keys()
{
    uint64_t sum = 0;

    for (int i = 0; i < (int)m_map.size(); i++) {
        sum += m_map[i].map->get_num_keys();
    }

    return sum;
}

/*============================================================================
 *                                get_size
 *============================================================================*/
uint64_t MemStore::get_size()
{
    uint64_t sum = 0;

    for (int i = 0; i < (int)m_map.size(); i++) {
        sum += m_map[i].map->get_size();
    }

    return sum;
}

/*============================================================================
 *                           get_size_when_serialized
 *============================================================================*/
uint64_t MemStore::get_size_when_serialized()
{
    uint64_t sum = 0;

    for (int i = 0; i < (int)m_map.size(); i++) {
        sum += m_map[i].map->get_size_when_serialized();
    }

    return sum;
}

/*============================================================================
 *                           will_reach_size_limit
 *============================================================================*/
bool MemStore::will_reach_size_limit(const char *key, const char *value, uint64_t timestamp)
{
    uint64_t sum = 0;

    for (int i = 0; i < (int)m_map.size(); i++) {
        sum += m_map[i].map->new_size(key, value, timestamp);
    }

    return (sum > m_maxsize);
}

/*============================================================================
 *                                   clear
 *============================================================================*/
void MemStore::clear()
{
    for (int i = 0; i < (int)m_map.size(); i++) {
        m_map[i].map->clear();
    }
}

/*============================================================================
 *                             new_map_inputstream
 *============================================================================*/
MapInputStream *MemStore::new_map_inputstream()
{
    assert(m_map.size() == 1);
    return new MapInputStream(m_map[0].map);
}

/*============================================================================
 *                             new_map_inputstream
 *============================================================================*/
MapInputStream *MemStore::new_map_inputstream(const char *key)
{
    return new MapInputStream(m_map[idx_of_map(key)].map);
}

/*============================================================================
 *                                 add_map
 *============================================================================*/
void MemStore::add_map(const char *key)
{
    int pos = idx_of_map(key);
    StrMapPair newpair;

    strcpy(newpair.key, key);
    newpair.map = new Map();

    if (m_map.size()) {
        pos++;
    }
    m_map.insert(m_map.begin() + pos, newpair);

    sanity_check();
}


/*============================================================================
 *                               get_num_maps
 *============================================================================*/
int MemStore::get_num_maps()
{
    return m_map.size();
}

/*============================================================================
 *                                 get_map
 *============================================================================*/
Map *MemStore::get_map(int idx)
{
    return m_map[idx].map;
}

/*============================================================================
 *                                 get_map
 *============================================================================*/
Map *MemStore::get_map(const char *key)
{
    return m_map[idx_of_map(key)].map;
}

/*============================================================================
 *                                 clear_map
 *============================================================================*/
void MemStore::clear_map(const char *key)
{
    get_map(key)->clear();
}

/*============================================================================
 *                                 idx_of_map
 *============================================================================*/
int MemStore::idx_of_map(const char *key)
{
    int first, last, mid, cmp;

    sanity_check();

    if (m_map.size() == 0) {
        return 0;
    }

    first = 0;
    last = m_map.size() - 1;
    while (first <= last) {
        mid = (last - first)/2 + first; // avoid overflow
        cmp = strcmp(key, m_map[mid].key);
        if (cmp > 0) {
            first = mid + 1;
        } else if (cmp < 0) {
            last = mid - 1;
        } else {
            break;
        }
    }

    if (strcmp(key, m_map[mid].key) < 0) {
        mid--;
    }

    // assert 'key' belongs to range [map[mid].key, map[mid+1].key)
    assert(strcmp(key, m_map[mid].key) >= 0 &&
           (mid == (int)m_map.size() - 1 || strcmp(key, m_map[mid+1].key) < 0));
    assert(mid >= 0 && mid < (int)m_map.size());

    return mid;
}

/*============================================================================
 *                                sanity_check
 *============================================================================*/
int MemStore::sanity_check()
{
#if DBGLVL > 1
    for (int i = 0; i < (int)m_map.size() - 1; i++) {
        assert(strcmp(m_map[i].key, m_map[i+1].key) < 0);
    }
#endif

    return 1;
}
