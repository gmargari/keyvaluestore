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
    // TODO: implement this
    *value = NULL;
    *timestamp = 0;
    return false;
}

/*=======================================================================*
 *                                  get
 *=======================================================================*/
bool DiskStore::get(const char *key, uint64_t timestamp, const char **value)
{
    uint64_t ts;

    if (get(key, value, &ts) && ts == timestamp) {
        return true;
    } else {
        *value = NULL;
        return false;
    }
}


/*========================================================================
 *                               num_keys
 *========================================================================*/
uint64_t DiskStore::num_keys()
{
    vector<KVTDiskFile *>::iterator iter;
    uint64_t total_keys = 0;

    for (iter = m_diskfiles.begin(); iter != m_diskfiles.end(); iter++) {
        total_keys += (*iter)->num_keys();
    }

    return total_keys;
}

/*========================================================================
 *                                 size
 *========================================================================*/
uint64_t DiskStore::size()
{
    vector<KVTDiskFile *>::iterator iter;
    uint64_t total_size = 0;

    for (iter = m_diskfiles.begin(); iter != m_diskfiles.end(); iter++) {
        total_size += (*iter)->size();
    }

    return total_size;
}

/*=======================================================================*
 *                              sanity_check
 *=======================================================================*/
void DiskStore::sanity_check()
{
    return_if_dbglvl_lt_2();
}
