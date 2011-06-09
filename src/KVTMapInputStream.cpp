#include "Global.h"
#include "KVTMapInputStream.h"

#include <cassert>
#include <cstdlib>

/*========================================================================
 *                           KVTMapInputStream
 *========================================================================*/
KVTMapInputStream::KVTMapInputStream(KVTMap *kvtmap)
{
    m_kvtmap = kvtmap;
    set_key_range(NULL, NULL, true, true);
    reset();
}

/*========================================================================
 *                          ~KVTMapInputStream
 *========================================================================*/
KVTMapInputStream::~KVTMapInputStream()
{
    if (m_start_key) {
        free(m_start_key);
    }

    if (m_end_key) {
        free(m_end_key);
    }
}

/*========================================================================
 *                             set_key_range
 *========================================================================*/
void KVTMapInputStream::set_key_range(const char *start_key, const char *end_key, bool start_incl, bool end_incl)
{
    if (start_key) {
        m_start_key = strdup(start_key);
    } else {
        m_start_key = NULL;
    }

    if (end_key) {
        m_end_key = strdup(end_key);
    } else {
        m_end_key = NULL;
    }

    m_start_incl = start_incl;
    m_end_incl = end_incl;
}

/*========================================================================
 *                             set_key_range
 *========================================================================*/
void KVTMapInputStream::set_key_range(const char *start_key, const char *end_key)
{
    set_key_range(start_key, end_key, true, false);
}

/*========================================================================
 *                                 reset
 *========================================================================*/
void KVTMapInputStream::reset()
{
    assert(!m_start_key || !m_end_key || strcmp(m_start_key, m_end_key) <= 0);
    m_iter = m_kvtmap->start_iter(m_start_key, m_start_incl);
    m_iter_end = m_kvtmap->end_iter(m_end_key, m_end_incl);
}

/*========================================================================
 *                                 read
 *========================================================================*/
bool KVTMapInputStream::read(const char **key, const char **value, uint64_t *timestamp)
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
        m_iter++;
        return true;
    }
}

/*=======================================================================*
 *                              sanity_check
 *=======================================================================*/
void KVTMapInputStream::sanity_check()
{
    return_if_dbglvl_lt_2();
}
