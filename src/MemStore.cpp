#include "Global.h"
#include "MemStore.h"

/*========================================================================
 *                               MemStore
 *========================================================================*/
MemStore::MemStore()
{
    m_maxsize = DEFAULT_MEMSTORE_SIZE;
    m_kvtmap = new KVTMap();
}

/*========================================================================
 *                              ~MemStore
 *========================================================================*/
MemStore::~MemStore()
{
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
bool MemStore::put(const char *key, const char *value)
{
    return m_kvtmap->put(key,value);
}

/*========================================================================
 *                                 get
 *========================================================================*/
const char *MemStore::get(const char *key)
{
    return m_kvtmap->get(key);
}

/*========================================================================
 *                               num_keys
 *========================================================================*/
uint64_t MemStore::num_keys()
{
    return m_kvtmap->num_keys();
}

/*========================================================================
 *                                 size
 *========================================================================*/
uint64_t MemStore::size()
{
    return m_kvtmap->size();
}

/*========================================================================
 *                                is_full
 *========================================================================*/
bool MemStore::is_full()
{
    return (size() > m_maxsize);
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
void MemStore::sanity_check()
{
    return_if_dbglvl_lt_2();
}
