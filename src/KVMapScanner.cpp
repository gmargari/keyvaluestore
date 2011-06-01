#include "Global.h"
#include "KVMapScanner.h"

#include <cassert>

/*========================================================================
 *                             KVMapScanner
 *========================================================================*/
KVMapScanner::KVMapScanner(KVMap *kvmap)
{
    m_kvmap = kvmap;
    iter = m_kvmap->m_map.begin();
}

/*========================================================================
 *                            ~KVMapScanner
 *========================================================================*/
KVMapScanner::~KVMapScanner()
{

}

/*========================================================================
 *                                 next
 *========================================================================*/
bool KVMapScanner::next(const char **key, const char **value)
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