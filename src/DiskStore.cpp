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
    vector<KVTDiskFile *>::iterator iter;

    for (iter = m_diskfiles.begin(); iter != m_diskfiles.end(); iter++) {
        delete *iter;
    }

    m_diskfiles.clear();
}

/*========================================================================
 *                                 get
 *========================================================================*/
bool DiskStore::get(const char *key, const char **value, uint64_t *timestamp)
{
    return false; // TODO: implement this
}

/*========================================================================
 *                               num_keys
 *========================================================================*/
uint64_t DiskStore::num_keys()
{
    return 0; // TODO: implement this
}

/*========================================================================
 *                                 size
 *========================================================================*/
uint64_t DiskStore::size()
{
    return 0; // TODO: implement this
}

/*=======================================================================*
 *                              sanity_check
 *=======================================================================*/
void DiskStore::sanity_check()
{
    return_if_dbglvl_lt_2();
}
