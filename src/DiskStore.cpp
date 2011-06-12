#include "Global.h"
#include "DiskStore.h"
#include "KVTDiskFileInputStream.h"

#include <cstdlib>
#include <cassert>

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
bool DiskStore::get(const char *key, char **value, uint64_t *timestamp)
{
    vector<KVTDiskFile *>::iterator iter;
    KVTDiskFileInputStream *istream;
    const char *k, *constvalue;

    // NOTE: TODO: every new file created by Compaction Manager should be inserted
    // at the *front* of the vector, so the first file in vector is the most
    // recent, the second is the second most recent etc.

    // TODO: have a protected vector of istreams, one for each kvtfile,
    // and use them to search. or else, create functions istream->set_vfile(),
    // istream->set_vfile_index() so we can create one input stream (with
    // its internal buffer) and use it to search all files.

    // search disk files in priority order. return the first value found,
    // since this is the most recent value
    for (iter = m_diskfiles.begin(); iter != m_diskfiles.end(); iter++) {
        istream = new KVTDiskFileInputStream(*iter);
        istream->set_key_range(key, key, true, true);
        if (istream->read(&k, &constvalue, timestamp)) {
            assert(strcmp(k, key) == 0);
            *value = strdup(constvalue); // copy value
            delete istream;
            return true;
        }
        delete istream;
    }

    *value = NULL;
    *timestamp = 0;
    return false;
}

/*=======================================================================*
 *                                  get
 *=======================================================================*/
bool DiskStore::get(const char *key, uint64_t timestamp, char **value)
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
