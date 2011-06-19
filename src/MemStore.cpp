#include "Global.h"
#include "MemStore.h"
#include "KVTMap.h"
#include "KVTMapInputStream.h"

/*========================================================================
 *                               MemStore
 *========================================================================*/
MemStore::MemStore()
{
    m_maxsize = DEFAULT_MEMSTORE_SIZE;
    m_kvtmap = new KVTMap();
    m_inputstream = new KVTMapInputStream(m_kvtmap);
}

/*========================================================================
 *                              ~MemStore
 *========================================================================*/
MemStore::~MemStore()
{
    delete m_inputstream;
    delete m_kvtmap;
}

/*========================================================================
 *                             set_maxsize
 *========================================================================*/
void MemStore::set_maxsize(uint64_t maxsize)
{
    m_maxsize = maxsize;
}

/*========================================================================
 *                             get_maxsize
 *========================================================================*/
uint64_t MemStore::get_maxsize(void)
{
    return m_maxsize;
}

/*========================================================================
 *                                 put
 *========================================================================*/
bool MemStore::put(const char *key, const char *value, uint64_t timestamp)
{
    return m_kvtmap->put(key, value, timestamp);
}

/*========================================================================
 *                                 put
 *========================================================================*/
bool MemStore::put(const char *key, const char *value)
{
    return m_kvtmap->put(key, value);
}

/*========================================================================
 *                                 get
 *========================================================================*/
bool MemStore::get(const char *key, char **value, uint64_t *timestamp)
{
    const char *constvalue;

    if (m_kvtmap->get(key, &constvalue, timestamp)) {
        *value = strdup(constvalue); // copy value
        return true;
    } else {
        return false;
    }
}


/*=======================================================================*
 *                                  get
 *=======================================================================*/
bool MemStore::get(const char *key, uint64_t timestamp, char **value)
{
    return get(key, timestamp, value);
}

/*========================================================================
 *                             get_num_keys
 *========================================================================*/
uint64_t MemStore::get_num_keys()
{
    return m_kvtmap->get_num_keys();
}

/*========================================================================
 *                               get_size
 *========================================================================*/
uint64_t MemStore::get_size()
{
    return m_kvtmap->get_size();
}

/*========================================================================
 *                                is_full
 *========================================================================*/
bool MemStore::is_full()
{
    return (get_size() > m_maxsize);
}

/*========================================================================
 *                                 clear
 *========================================================================*/
void MemStore::clear()
{
    m_kvtmap->clear();
}

/*=======================================================================*
 *                              sanity_check
 *=======================================================================*/
int MemStore::sanity_check()
{
    return 1;
}
