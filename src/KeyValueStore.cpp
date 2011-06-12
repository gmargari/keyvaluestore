#include "Global.h"
#include "KeyValueStore.h"
#include "MemStore.h"
#include "DiskStore.h"
#include "CompactionManager.h"

#include <cassert>

/*========================================================================
 *                            KeyValueStore
 *========================================================================*/
KeyValueStore::KeyValueStore()
{
    m_memstore = new MemStore();
    m_diskstore = new DiskStore();
    m_compactionmanager = new CompactionManager(m_memstore, m_diskstore);
    set_memstore_maxsize(DEFAULT_MEMSTORE_SIZE);
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
 *                         set_memstore_maxsize
 *========================================================================*/
void KeyValueStore::set_memstore_maxsize(uint64_t maxsize)
{
    m_memstore->set_maxsize(maxsize);
}

/*========================================================================
 *                         set_memstore_maxsize
 *========================================================================*/
uint64_t KeyValueStore::get_memstore_maxsize()
{
    return m_memstore->get_maxsize();
}

/*========================================================================
 *                                 put
 *========================================================================*/
bool KeyValueStore::put(const char *key, const char *value, uint64_t timestamp)
{
    if (m_memstore->is_full()) {
        m_compactionmanager->flush_memstore();
    }

    return m_memstore->put(key, value, timestamp);
}

/*========================================================================
 *                                 put
 *========================================================================*/
bool KeyValueStore::put(const char *key, const char *value)
{
    if (m_memstore->is_full()) {
        m_compactionmanager->flush_memstore();
    }

    return m_memstore->put(key, value);
}

/*========================================================================
 *                                 get
 *========================================================================*/
bool KeyValueStore::get(const char *key, char **value, uint64_t *timestamp)
{
    // if key found in memstore, return since this is the most recent value:
    if (m_memstore->get(key, value, timestamp)) {
        return true;
    }
    // else, search in diskstore:
    else {
        return m_diskstore->get(key, value, timestamp);
    }
}

/*========================================================================
 *                                 get
 *========================================================================*/
bool KeyValueStore::get(const char *key, uint64_t timestamp, char **value)
{
    if (m_memstore->get(key, timestamp, value)) {
        return true;
    } else {
        return m_diskstore->get(key, timestamp, value);
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

// TODO: move elsewhere
/*=======================================================================*
 *                           check_parameters
 *=======================================================================*/
void KeyValueStore::check_parameters()
{
    // need at least these bytes (e.g. to fully decode a kvt read from disk:
    // <keylen, valuelen, key, value, timestamp>
    assert(SCANNERBUFSIZE >= (2 * sizeof(uint64_t) + 2 * MAX_KVTSIZE + sizeof(uint64_t)));
}
