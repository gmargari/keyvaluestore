#include "Global.h"
#include "MemStore.h"

#include "Map.h"
#include "MapInputStream.h"

/*============================================================================
 *                                 MemStore
 *============================================================================*/
MemStore::MemStore()
    : m_map(), m_maxsize(DEFAULT_MEMSTORE_SIZE)
{
    m_map = new Map();
}

/*============================================================================
 *                                ~MemStore
 *============================================================================*/
MemStore::~MemStore()
{
    delete m_map;
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
    return m_map->put(key, value, timestamp);
}

/*============================================================================
 *                                   get
 *============================================================================*/
bool MemStore::get(const char *key, char **value, uint64_t *timestamp)
{
    const char *constvalue;

    if (m_map->get(key, &constvalue, timestamp)) {
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
    return m_map->get_num_keys();
}

/*============================================================================
 *                                get_size
 *============================================================================*/
uint64_t MemStore::get_size()
{
    return m_map->get_size();
}

/*============================================================================
 *                                get_size
 *============================================================================*/
uint64_t MemStore::get_size(const char *start_key, const char *end_key, bool start_key_incl, bool end_key_incl)
{
    return m_map->get_size(start_key, end_key, start_key_incl, end_key_incl);
}

/*============================================================================
 *                           get_size_when_serialized
 *============================================================================*/
uint64_t MemStore::get_size_when_serialized()
{
    return m_map->get_size_when_serialized();
}

/*============================================================================
 *                           get_size_when_serialized
 *============================================================================*/
uint64_t MemStore::get_size_when_serialized(const char *start_key, const char *end_key, bool start_key_incl, bool end_key_incl)
{
    return m_map->get_size_when_serialized(start_key, end_key, true, false);
}

/*============================================================================
 *                                 get_sizes
 *============================================================================*/
pair<uint64_t, uint64_t> MemStore::get_sizes()
{
    return m_map->get_sizes();
}

/*============================================================================
 *                                 get_sizes
 *============================================================================*/
pair<uint64_t, uint64_t> MemStore::get_sizes(const char *start_key, const char *end_key, bool start_key_incl, bool end_key_incl)
{
    return m_map->get_sizes(start_key, end_key, start_key_incl, end_key_incl);
}

/*============================================================================
 *                           will_reach_size_limit
 *============================================================================*/
bool MemStore::will_reach_size_limit(const char *key, const char *value, uint64_t timestamp)
{
    return (m_map->new_size(key, value, timestamp) > m_maxsize);
}

/*============================================================================
 *                           will_reach_size_limit
 *============================================================================*/
bool MemStore::will_reach_size_limit(const char *key, const char *value)
{
    return will_reach_size_limit(key, value, 0);
}

/*============================================================================
 *                                   clear
 *============================================================================*/
void MemStore::clear()
{
    m_map->clear();
}

/*============================================================================
 *                                   clear
 *============================================================================*/
void MemStore::clear(const char *start_key, const char *end_key, bool start_key_incl, bool end_key_incl)
{
    m_map->clear(start_key, end_key, start_key_incl, end_key_incl);
}
