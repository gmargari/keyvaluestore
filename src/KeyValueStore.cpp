#include "Global.h"
#include "KeyValueStore.h"

#include "MemStore.h"
#include "DiskStore.h"
#include "NomergeCompactionManager.h"
#include "ImmCompactionManager.h"
#include "LogCompactionManager.h"
#include "GeomCompactionManager.h"
#include "RangemergeCompactionManager.h"
#include "CassandraCompactionManager.h"
#include "Statistics.h"

#include <cassert>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>

using namespace std;

/*============================================================================
 *                               KeyValueStore
 *============================================================================*/
KeyValueStore::KeyValueStore(cm_type type)
    : m_memstore(), m_diskstore(), m_compactionmanager()
{
    m_memstore = new MemStore();
    m_diskstore = new DiskStore();
    switch (type) {
        case KeyValueStore::NOMERGE_CM:
            m_compactionmanager = new NomergeCompactionManager(m_memstore, m_diskstore);
            break;
        case KeyValueStore::IMM_CM:
            m_compactionmanager = new ImmCompactionManager(m_memstore, m_diskstore);
            break;
        case KeyValueStore::GEOM_CM:
            m_compactionmanager = new GeomCompactionManager(m_memstore, m_diskstore);
            break;
        case KeyValueStore::LOG_CM:
            m_compactionmanager = new LogCompactionManager(m_memstore, m_diskstore);
            break;
        case KeyValueStore::RNGMERGE_CM:
            m_compactionmanager = new RangemergeCompactionManager(m_memstore, m_diskstore);
            break;
        case KeyValueStore::CASSANDRA_CM:
            m_compactionmanager = new CassandraCompactionManager(m_memstore, m_diskstore);
            break;
    }
    set_memstore_maxsize(DEFAULT_MEMSTORE_SIZE);
    check_parameters();
    global_stats_init();
}

/*============================================================================
 *                              ~KeyValueStore
 *============================================================================*/
KeyValueStore::~KeyValueStore()
{
    delete m_memstore;
    delete m_diskstore;
    delete m_compactionmanager;
}

/*============================================================================
 *                           set_memstore_maxsize
 *============================================================================*/
void KeyValueStore::set_memstore_maxsize(uint64_t maxsize)
{
    m_memstore->set_maxsize(maxsize);
}

/*============================================================================
 *                           set_memstore_maxsize
 *============================================================================*/
uint64_t KeyValueStore::get_memstore_maxsize()
{
    return m_memstore->get_maxsize();
}

/*============================================================================
 *                                   put
 *============================================================================*/
bool KeyValueStore::put(const char *key, const char *value, uint64_t timestamp)
{
    assert(m_memstore->get_size() <= m_memstore->get_maxsize());
    if (m_memstore->will_reach_size_limit(key, value, timestamp)) {
        time_start(&(g_stats.compaction_time));
        flush_bytes();
        time_end(&(g_stats.compaction_time));
        g_stats.disk_files = m_diskstore->get_num_disk_files();

        print_stats();
    }

    return m_memstore->put(key, value, timestamp);
}

/*============================================================================
 *                                   put
 *============================================================================*/
bool KeyValueStore::put(const char *key, const char *value)
{
    return put(key, value, get_timestamp());
}

/*============================================================================
 *                             get_num_mem_keys
 *============================================================================*/
uint64_t KeyValueStore::get_num_mem_keys()
{
    return m_memstore->get_num_keys();
}

/*============================================================================
 *                             get_num_disk_keys
 *============================================================================*/
uint64_t KeyValueStore::get_num_disk_keys()
{
    return m_diskstore->get_num_keys();
}

/*============================================================================
 *                               get_mem_size
 *============================================================================*/
uint64_t KeyValueStore::get_mem_size()
{
    return m_memstore->get_size();
}

/*============================================================================
 *                               get_disk_size
 *============================================================================*/
uint64_t KeyValueStore::get_disk_size()
{
    return m_diskstore->get_size();
}

/*============================================================================
 *                             get_num_disk_files
 *============================================================================*/
int KeyValueStore::get_num_disk_files()
{
    return m_diskstore->get_num_disk_files();
}

/*============================================================================
 *                                flush_bytes
 *============================================================================*/
void KeyValueStore::flush_bytes()
{
    m_compactionmanager->flush_bytes();
}

/*============================================================================
 *                          get_compaction_manager
 *============================================================================*/
CompactionManager *KeyValueStore::get_compaction_manager()
{
    return m_compactionmanager;
}

/*============================================================================
 *                               get_timestamp
 *============================================================================*/
uint64_t KeyValueStore::get_timestamp()
{
//    struct timeval tv;
    static uint64_t t = 0;

//     gettimeofday(&tv, NULL);
//     return (uint64_t)(tv.tv_sec*1000000 + tv.tv_usec);
    return ++t;
}

// TODO: move elsewhere
/*============================================================================
 *                            check_parameters
 *============================================================================*/
void KeyValueStore::check_parameters()
{
    struct stat st;

    if (stat(ROOT_DIR, &st) != 0) {
        cout << "Error: " << ROOT_DIR << " does not exist" << endl;
        exit(EXIT_FAILURE);
    }

    // need at least these bytes (e.g. to fully decode a <k,v,t> read from disk:
    // <keylen, valuelen, key, value, timestamp>)
    assert(MERGE_BUFSIZE >= (2 * sizeof(uint64_t) + 2 * MAX_KVTSIZE + sizeof(uint64_t)));
}
