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
    
    if (m_start_key) {
        m_iter = m_kvmap->m_map.lower_bound(m_start_key);
        if (m_start_incl == false && strcmp(m_iter->first, m_start_key) == 0) {
            m_iter++;
        }
    } else {
        m_iter = m_kvmap->m_map.begin();
    }

    if (m_end_key) {
        // upper_bound(x) returns an iterator pointing to the first element 
        // whose key compares strictly greater than x
        m_iter_end = m_kvmap->m_map.upper_bound(m_end_key);
        
        // if 'end_key' is not inclusive, set 'm_iter_end' one position back
        // and check if 'm_iter_end' has key equal to 'end_key'. if yes, 
        // leave it there, else forward it one position.
        if (m_end_incl == false && m_iter_end != m_iter) {
            m_iter_end--;
            if (strcmp(m_iter_end->first, m_end_key) != 0) {
                m_iter_end++;
            }
        }
    } else {
        m_iter_end = m_kvmap->m_map.end();
    }
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
