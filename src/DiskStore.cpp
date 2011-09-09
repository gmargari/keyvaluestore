#include "Global.h"
#include "DiskStore.h"

#include "DiskFile.h"
#include "DiskFileInputStream.h"

#include <cstdlib>
#include <cassert>
#include <cstring>

using namespace std;

/*============================================================================
 *                                 DiskStore
 *============================================================================*/
DiskStore::DiskStore()
{

}

/*============================================================================
 *                                ~DiskStore
 *============================================================================*/
DiskStore::~DiskStore()
{
    for (int i = 0; i < (int)m_disk_files.size(); i++) {
        delete m_disk_files[i];
    }
    m_disk_files.clear();

    for (int i = 0; i < (int)m_disk_istreams.size(); i++) {
        delete m_disk_istreams[i];
    }
    m_disk_istreams.clear();
}

/*============================================================================
 *                                   get
 *============================================================================*/
bool DiskStore::get(const char *key, char **value, uint64_t *timestamp)
{
    DiskFileInputStream *disk_istream;
    const char *k, *constvalue;

    // search disk files in order, from most recently created to oldest.
    // return the first value found, since this is the most recent value
    assert(m_disk_files.size() == m_disk_istreams.size());
    for (int i = 0; i < (int)m_disk_files.size(); i++) {
        disk_istream = m_disk_istreams[i];
        disk_istream->set_key_range(key, key, true, true);
        if (disk_istream->read(&k, &constvalue, timestamp)) {
            assert(strcmp(k, key) == 0);
            *value = strdup(constvalue); // copy value
            return true;
        }
    }

    *value = NULL;
    *timestamp = 0;
    return false;
}

/*============================================================================
 *                                   get
 *============================================================================*/
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

/*============================================================================
 *                               get_num_keys
 *============================================================================*/
uint64_t DiskStore::get_num_keys()
{
    uint64_t total_keys = 0;

    for (int i = 0; i < (int)m_disk_files.size(); i++) {
        total_keys += m_disk_files[i]->get_num_keys();
    }

    return total_keys;
}

/*============================================================================
 *                                get_size
 *============================================================================*/
uint64_t DiskStore::get_size()
{
    uint64_t total_size = 0;

    for (int i = 0; i < (int)m_disk_files.size(); i++) {
        total_size += m_disk_files[i]->get_size();
    }

    return total_size;
}

/*============================================================================
 *                            get_num_disk_files
 *============================================================================*/
int DiskStore::get_num_disk_files()
{
    return m_disk_files.size();
}
