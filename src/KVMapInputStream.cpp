#include "Global.h"
#include "KVMapInputStream.h"

#include <cassert>
#include <cstdlib>

/*========================================================================
 *                           KVMapInputStream
 *========================================================================*/
KVMapInputStream::KVMapInputStream(KVMap *kvmap)
{
    init(kvmap, NULL, NULL, true, true);
    reset();
}

/*========================================================================
 *                           KVMapInputStream
 *========================================================================*/
KVMapInputStream::KVMapInputStream(KVMap *kvmap, const char *start_key, const char *end_key)
{
    init(kvmap, start_key, end_key, true, false);
    reset();
}

/*========================================================================
 *                           KVMapInputStream
 *========================================================================*/
KVMapInputStream::KVMapInputStream(KVMap *kvmap, const char *start_key, const char *end_key, bool start_incl, bool end_incl)
{
    init(kvmap, start_key, end_key, start_incl, end_incl);
    reset();
}

/*========================================================================
 *                                init
 *========================================================================*/
void KVMapInputStream::init(KVMap *kvmap, const char *start_key, const char *end_key, bool start_incl, bool end_incl)
{
    m_kvmap = kvmap;
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
 *                          ~KVMapInputStream
 *========================================================================*/
KVMapInputStream::~KVMapInputStream()
{
    if (m_start_key) {
        free(m_start_key);
    }
    if (m_end_key) {
        free(m_end_key);
    }
}

/*========================================================================
 *                                 reset
 *========================================================================*/
void KVMapInputStream::reset()
{
    assert(!m_start_key || !m_end_key || strcmp(m_start_key, m_end_key) <= 0);
    m_iter = m_kvmap->start_iter(m_start_key, m_start_incl);
    m_iter_end = m_kvmap->end_iter(m_end_key, m_end_incl);
}

#include <cstdio>

/*========================================================================
 *                                 read
 *========================================================================*/
bool KVMapInputStream::read(const char **key, const char **value)
{
    assert(key && value);
    
    if (m_iter == m_iter_end) {
        *key = NULL;
        *value = NULL;
        return false;
    } else {
        *key = m_iter->first;
        *value = m_iter->second;
        m_iter++;
        return true;
    }
}

/*=======================================================================*
 *                              sanity_check
 *=======================================================================*/
void KVMapInputStream::sanity_check()
{
#if DBGLVL < 2
    return;
#endif
}
