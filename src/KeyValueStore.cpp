#include "Global.h"
#include "KeyValueStore.h"
#include "MemStore.h"
#include "DiskStore.h"

/*========================================================================
 *                            KeyValueStore
 *========================================================================*/
KeyValueStore::KeyValueStore()
{
    m_memstore = new MemStore();
    m_diskstore = new DiskStore();
    m_compactionmanager = new CompactionManager(m_memstore, m_diskstore);
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

#include <stdio.h>

/*========================================================================
 *                                 put
 *========================================================================*/
bool KeyValueStore::put(const char *key, const char *value)
{
    if (m_memstore->num_keys() > 4) {
        m_compactionmanager->flush_memstore();
    }
    
    return m_memstore->put(key,value);
}

/*========================================================================
 *                                 get
 *========================================================================*/
char *KeyValueStore::get(char *key)
{
    char *value;
    
    if ((value = m_memstore->get(key))) {
        return value;
    } else {
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

// TEST
void KeyValueStore::dumpmem()
{
    m_compactionmanager->flush_memstore();
}

//TEST
void KeyValueStore::catdiskfiles()
{
    m_compactionmanager->catdiskfiles();
}