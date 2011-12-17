// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#include "./Global.h"
#include "./MapInputStream.h"

#include <assert.h>

/*============================================================================
 *                              MapInputStream
 *============================================================================*/
MapInputStream::MapInputStream(Map *map)
    : m_map(map), m_iter(), m_iter_end(), m_start_key(), m_end_key(),
      m_start_incl(true), m_end_incl(true) {
    set_key_range(Slice(NULL, 0), Slice(NULL, 0));
}

/*============================================================================
 *                             ~MapInputStream
 *============================================================================*/
MapInputStream::~MapInputStream() {
}

/*============================================================================
 *                              set_key_range
 *============================================================================*/
void MapInputStream::set_key_range(Slice start_key, Slice end_key,
                                   bool start_incl, bool end_incl) {
    m_start_key = start_key;
    m_end_key = end_key;
    m_start_incl = start_incl;
    m_end_incl = end_incl;

    assert(!m_start_key.data() || !m_end_key.data()
             || strcmp(m_start_key.data(), m_end_key.data()) <= 0);
    assert(m_map);
    m_iter = m_map->start_iter(m_start_key, m_start_incl);
    m_iter_end = m_map->end_iter(m_end_key, m_end_incl);
}

/*============================================================================
 *                              set_key_range
 *============================================================================*/
void MapInputStream::set_key_range(Slice start_key, Slice end_key) {
    set_key_range(start_key, end_key, true, false);
}

/*============================================================================
 *                                  read
 *============================================================================*/
bool MapInputStream::read(Slice *key, Slice *value, uint64_t *timestamp) {
    assert(key && value && timestamp);

    if (m_iter == m_iter_end) {
        *key = Slice(NULL, 0);
        *value = Slice(NULL, 0);
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
