// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#include "./MapInputStream.h"

#include <assert.h>

/*============================================================================
 *                              MapInputStream
 *============================================================================*/
MapInputStream::MapInputStream(Map *map)
    : m_map(map), m_iter(), m_iter_end(), m_start_key(NULL), m_start_keylen(0),
      m_end_key(NULL), m_end_keylen(0), m_start_incl(true), m_end_incl(true) {
    set_key_range(NULL, 0, NULL, 0);
}

/*============================================================================
 *                             ~MapInputStream
 *============================================================================*/
MapInputStream::~MapInputStream() {
}

/*============================================================================
 *                              set_key_range
 *============================================================================*/
void MapInputStream::set_key_range(const char *start_key, uint32_t start_keylen, const char *end_key, uint32_t end_keylen, bool start_incl, bool end_incl) {
    m_start_key = start_key;
    m_start_keylen = start_keylen;
    m_end_key = end_key;
    m_end_keylen = end_keylen;
    m_start_incl = start_incl;
    m_end_incl = end_incl;

    assert(!m_start_key || !m_end_key || strcmp(m_start_key, m_end_key) <= 0);
    assert(m_map);
    m_iter = m_map->start_iter(m_start_key, start_keylen, m_start_incl);
    m_iter_end = m_map->end_iter(m_end_key, end_keylen, m_end_incl);
}

/*============================================================================
 *                              set_key_range
 *============================================================================*/
void MapInputStream::set_key_range(const char *start_key, uint32_t start_keylen, const char *end_key, uint32_t end_keylen) {
    set_key_range(start_key, start_keylen, end_key, end_keylen, true, false);
}

/*============================================================================
 *                                  read
 *============================================================================*/
bool MapInputStream::read(const char **key, uint32_t *keylen, const char **value, uint32_t *valuelen, uint64_t *timestamp) {
    assert(key && value && timestamp);

    if (m_iter == m_iter_end) {
        *key = NULL;
        *keylen = 0;
        *value = NULL;
        *valuelen = 0;
        *timestamp = 0;
        return false;
    } else {
        *key = m_iter->first.data();
        *keylen = m_iter->first.size();
        *value = m_iter->second.first.data();
        *valuelen = m_iter->second.first.size();
        *timestamp = m_iter->second.second;
        ++m_iter;
        return true;
    }
}
