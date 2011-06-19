#include "Global.h"
#include "KeyValueStore.h"
#include "MemStore.h"
#include "DiskStore.h"
#include "ImmCompactionManager.h"
#include "LogCompactionManager.h"
#include "GeomCompactionManager.h"
#include "UrfCompactionManager.h"

#include <cassert>

/*========================================================================
 *                            KeyValueStore
 *========================================================================*/
KeyValueStore::KeyValueStore(cm_type type)
{
    m_memstore = new MemStore();
    m_diskstore = new DiskStore();
    switch (type) {
        case KeyValueStore::IMM_CM:
            m_compactionmanager = new ImmCompactionManager(m_memstore, m_diskstore);
            break;
        case KeyValueStore::GEOM_CM:
            m_compactionmanager = new GeomCompactionManager(m_memstore, m_diskstore);
            break;
        case KeyValueStore::LOG_CM:
            m_compactionmanager = new LogCompactionManager(m_memstore, m_diskstore);
            break;
        case KeyValueStore::URF_CM:
            m_compactionmanager = new UrfCompactionManager(m_memstore, m_diskstore);
            break;
    }
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
        m_compactionmanager->flush_bytes();
    }

    return m_memstore->put(key, value, timestamp);
}

/*========================================================================
 *                                 put
 *========================================================================*/
bool KeyValueStore::put(const char *key, const char *value)
{
    if (m_memstore->is_full()) {
        m_compactionmanager->flush_bytes();
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
 *                           get_num_mem_keys
 *========================================================================*/
uint64_t KeyValueStore::get_num_mem_keys()
{
    return m_memstore->get_num_keys();
}

/*========================================================================
 *                           get_num_disk_keys
 *========================================================================*/
uint64_t KeyValueStore::get_num_disk_keys()
{
    return m_diskstore->get_num_keys();
}

/*========================================================================
 *                             get_mem_size
 *========================================================================*/
uint64_t KeyValueStore::get_mem_size()
{
    return m_memstore->get_size();
}

/*========================================================================
 *                            get_disk_size
 *========================================================================*/
uint64_t KeyValueStore::get_disk_size()
{
    return m_diskstore->get_size();
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

/*=======================================================================*
 *                             sanity_check
 *=======================================================================*/
int KeyValueStore::sanity_check()
{
    return 1;
}
