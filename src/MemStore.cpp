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
 *                               get_size
 *========================================================================*/
uint64_t MemStore::get_size(const char *start_key, const char *end_key)
{
    return m_kvtmap->get_size(start_key, end_key);
}

/*========================================================================
 *                               get_size
 *========================================================================*/
uint64_t MemStore::get_size(const char *start_key, const char *end_key, bool start_key_incl, bool end_key_incl)
{
    return m_kvtmap->get_size(start_key, end_key, start_key_incl, end_key_incl);
}

/*========================================================================
 *                         will_reach_size_limit
 *========================================================================*/
bool MemStore::will_reach_size_limit(const char *key, const char *value, uint64_t timestamp)
{
    return (m_kvtmap->new_size(key, value, timestamp) > m_maxsize);
}

/*========================================================================
 *                         will_reach_size_limit
 *========================================================================*/
bool MemStore::will_reach_size_limit(const char *key, const char *value)
{
    return will_reach_size_limit(key, value, 0);
}

/*========================================================================
 *                                 clear
 *========================================================================*/
void MemStore::clear()
{
    m_kvtmap->clear();
}

/*========================================================================
 *                                 clear
 *========================================================================*/
void MemStore::clear(const char *start_key, const char *end_key)
{
    m_kvtmap->clear(start_key, end_key);
}

/*========================================================================
 *                                 clear
 *========================================================================*/
void MemStore::clear(const char *start_key, const char *end_key, bool start_key_incl, bool end_key_incl)
{
    m_kvtmap->clear(start_key, end_key, start_key_incl, end_key_incl);
}

/*=======================================================================*
 *                              sanity_check
 *=======================================================================*/
int MemStore::sanity_check()
{
    return 1;
}
