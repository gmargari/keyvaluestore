#include "Global.h"
#include "MapInputStream.h"

#include <cassert>
#include <cstdlib>

/*============================================================================
 *                              MapInputStream
 *============================================================================*/
MapInputStream::MapInputStream(Map *map)
    : m_map(map), m_iter(), m_iter_end(), m_start_key(NULL), m_end_key(NULL),
      m_start_incl(true), m_end_incl(true)
{
    set_key_range(NULL, NULL);
}

/*============================================================================
 *                             ~MapInputStream
 *============================================================================*/
MapInputStream::~MapInputStream()
{

}

/*============================================================================
 *                              set_key_range
 *============================================================================*/
void MapInputStream::set_key_range(const char *start_key, const char *end_key, bool start_incl, bool end_incl)
{
    m_start_key = start_key;
    m_end_key = end_key;
    m_start_incl = start_incl;
    m_end_incl = end_incl;

    assert(!m_start_key || !m_end_key || strcmp(m_start_key, m_end_key) <= 0);
    m_iter = m_map->start_iter(m_start_key, m_start_incl);
    m_iter_end = m_map->end_iter(m_end_key, m_end_incl);
}

/*============================================================================
 *                              set_key_range
 *============================================================================*/
void MapInputStream::set_key_range(const char *start_key, const char *end_key)
{
    set_key_range(start_key, end_key, true, false);
}

/*============================================================================
 *                                  read
 *============================================================================*/
bool MapInputStream::read(const char **key, const char **value, uint64_t *timestamp)
{
    assert(key && value && timestamp);

    if (m_iter == m_iter_end) {
        *key = NULL;
        *value = NULL;
        *timestamp = 0;
        return false;
    } else {
        *key = m_iter->first;
        *value = m_iter->second.first;
        *timestamp = m_iter->second.second;
        ++m_iter;
        return true;
    }
}
