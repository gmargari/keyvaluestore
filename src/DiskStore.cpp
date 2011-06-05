#include "Global.h"
#include "DiskStore.h"

#include <cstdlib>

using namespace std;

/*========================================================================
 *                              DiskStore
 *========================================================================*/
DiskStore::DiskStore()
{

}

/*========================================================================
 *                             ~DiskStore
 *========================================================================*/
DiskStore::~DiskStore()
{
    vector<KVDiskFile *>::iterator iter;
    
    for (iter = m_diskfiles.begin(); iter != m_diskfiles.end(); iter++) {
        delete *iter;
    }
    
    m_diskfiles.clear();
}

/*========================================================================
 *                                 get
 *========================================================================*/
const char *DiskStore::get(const char *key)
{
    return NULL;
}

/*========================================================================
 *                               num_keys
 *========================================================================*/
uint64_t DiskStore::num_keys()
{
    return 0;
}

/*========================================================================
 *                                 size
 *========================================================================*/
uint64_t DiskStore::size()
{
    return 0;
}

/*=======================================================================*
 *                              sanity_check
 *=======================================================================*/
void DiskStore::sanity_check()
{
#if DBGLVL < 2
    return;
#endif
}
