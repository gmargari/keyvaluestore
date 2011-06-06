#include "Global.h"
#include "KeyValueStore.h"
#include "MemStore.h"
#include "DiskStore.h"

#include <cassert>

/*========================================================================
 *                            KeyValueStore
 *========================================================================*/
KeyValueStore::KeyValueStore()
{
    m_memstore = new MemStore();
    m_memstore->set_maxsize(10000);
    m_diskstore = new DiskStore();
    m_compactionmanager = new CompactionManager(m_memstore, m_diskstore);

    check_parameters();
}

/*========================================================================
 *                           ~KeyValueStore
 *========================================================================*/
KeyValueStore::~KeyValueStore()
{
    delete m_memstore;
    delete m_diskstore;
    delete m_compactionmanager;
}

/*========================================================================
 *                                 put
 *========================================================================*/
bool KeyValueStore::put(const char *key, const char *value)
{
    if (m_memstore->is_full()) {
        m_compactionmanager->flush_memstore();
    }

    return m_memstore->put(key,value);
}

/*========================================================================
 *                                 get
 *========================================================================*/
const char *KeyValueStore::get(const char *key)
{
    const char *value;

    // if key found in memstore return, since this is the most recent value
    if ((value = m_memstore->get(key))) {
        return value;
    }
    // else, search in diskstore
    else {
        return m_diskstore->get(key);
    }
}

/*========================================================================
 *                             num_mem_keys
 *========================================================================*/
uint64_t KeyValueStore::num_mem_keys()
{
    return m_memstore->num_keys();
}

/*========================================================================
 *                             num_disk_keys
 *========================================================================*/
uint64_t KeyValueStore::num_disk_keys()
{
    return m_diskstore->num_keys();
}

/*========================================================================
 *                               mem_size
 *========================================================================*/
uint64_t KeyValueStore::mem_size()
{
    return m_memstore->size();
}

/*========================================================================
 *                              disk_size
 *========================================================================*/
uint64_t KeyValueStore::disk_size()
{
    return m_diskstore->size();
}

/*=======================================================================*
 *                             sanity_check
 *=======================================================================*/
void KeyValueStore::sanity_check()
{
    return_if_dbglvl_lt_2();
}

/*=======================================================================*
 *                           check_parameters
 *=======================================================================*/
void KeyValueStore::check_parameters()
{
    assert(SCANNERBUFSIZE >= (2*MAX_KVSIZE + 2*sizeof(uint64_t))); // need at least these bytes (e.g. to fully decode a kv read from disk)
}
