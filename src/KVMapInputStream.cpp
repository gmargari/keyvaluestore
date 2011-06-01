#include "Global.h"
#include "KVMapInputStream.h"

#include <cassert>

/*========================================================================
 *                           KVMapInputStream
 *========================================================================*/
KVMapInputStream::KVMapInputStream(KVMap *kvmap)
{
    m_kvmap = kvmap;
    iter = m_kvmap->m_map.begin();
}

/*========================================================================
 *                          ~KVMapInputStream
 *========================================================================*/
KVMapInputStream::~KVMapInputStream()
{

}

/*========================================================================
 *                                 reset
 *========================================================================*/
void KVMapInputStream::reset()
{
    iter = m_kvmap->m_map.begin();
}

/*========================================================================
 *                                 read
 *========================================================================*/
bool KVMapInputStream::read(const char **key, const char **value)
{
    assert(key && value);
    if (iter == m_kvmap->m_map.end()) {
        *key = NULL;
        *value = NULL;
        return false;
    } else {
        *key = (*iter).first;
        *value = (*iter).second;
        iter++;
        return true;
    }
}