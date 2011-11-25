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
#include <cstdio>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>

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
    assert(m_memstore->get_size() < m_memstore->get_maxsize());
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
 *                         set_memstore_merge_type
 *============================================================================*/
void KeyValueStore::set_memstore_merge_type(merge_type type)
{
    m_compactionmanager->set_memstore_merge_type(type);
}

/*============================================================================
 *                         get_memstore_merge_type
 *============================================================================*/
merge_type KeyValueStore::get_memstore_merge_type()
{
    return m_compactionmanager->get_memstore_merge_type();
}

/*============================================================================
 *                             get_mb_inserted
 *============================================================================*/
uint32_t KeyValueStore::get_mb_inserted()
{
    return gb2mb(bytes_get_gb(g_stats.bytes_inserted)) + bytes_get_mb(g_stats.bytes_inserted);
}

/*============================================================================
 *                               get_mb_read
 *============================================================================*/
uint32_t KeyValueStore::get_mb_read()
{
    return gb2mb(bytes_get_gb(g_stats.bytes_read)) + bytes_get_mb(g_stats.bytes_read);
}

/*============================================================================
 *                               get_mb_read
 *============================================================================*/
uint32_t KeyValueStore::get_mb_written()
{
    return gb2mb(bytes_get_gb(g_stats.bytes_written)) + bytes_get_mb(g_stats.bytes_written);
}

/*============================================================================
 *                               get_mb_read
 *============================================================================*/
uint32_t KeyValueStore::get_num_reads()
{
    return g_stats.num_reads;
}

/*============================================================================
 *                             get_num_writes
 *============================================================================*/
uint32_t KeyValueStore::get_num_writes()
{
    return g_stats.num_writes;
}

/*============================================================================
 *                            get_total_time_sec
 *============================================================================*/
uint32_t KeyValueStore::get_total_time_sec()
{
    time_end(&(g_stats.total_time));    // NOTE: this should not be done here!
    time_start(&(g_stats.total_time));  // the problem is that get_total_time_sec() must be called first, in order to update properly total_time, and then call rest functions

    return time_get_secs(g_stats.total_time);
}

/*============================================================================
 *                            get_put_time_sec
 *============================================================================*/
uint32_t KeyValueStore::get_put_time_sec()
{
    int32_t time = time_get_secs(g_stats.total_time) - get_compaction_time_sec();

    return (time < 0 ? 0 : time );
}

/*============================================================================
 *                          get_compaction_time_sec
 *============================================================================*/
uint32_t KeyValueStore::get_compaction_time_sec()
{
    return time_get_secs(g_stats.compaction_time);
}

/*============================================================================
 *                           get_merge_time_sec
 *============================================================================*/
uint32_t KeyValueStore::get_merge_time_sec()
{
    return time_get_secs(g_stats.merge_time);
}

/*============================================================================
 *                           get_free_time_sec
 *============================================================================*/
uint32_t KeyValueStore::get_free_time_sec()
{
    return time_get_secs(g_stats.free_time);
}

/*============================================================================
 *                          get_cmrest_time_sec
 *============================================================================*/
uint32_t KeyValueStore::get_cmrest_time_sec()
{
    int32_t time = get_compaction_time_sec() - get_merge_time_sec() - get_free_time_sec();

    return (time < 0 ? 0 : time );
}

/*============================================================================
 *                           get_mem_time_sec
 *============================================================================*/
uint32_t KeyValueStore::get_mem_time_sec()
{
    int32_t time = get_merge_time_sec() - get_read_time_sec() - get_write_time_sec();

    return (time < 0 ? 0 : time );
}

/*============================================================================
 *                            get_io_time_sec
 *============================================================================*/
uint32_t KeyValueStore::get_io_time_sec()
{
    return get_read_time_sec() + get_write_time_sec();
}

/*============================================================================
 *                           get_read_time_sec
 *============================================================================*/
uint32_t KeyValueStore::get_read_time_sec()
{
    return time_get_secs(g_stats.read_time);
}

/*============================================================================
 *                           get_write_time_sec
 *============================================================================*/
uint32_t KeyValueStore::get_write_time_sec()
{
    return time_get_secs(g_stats.write_time);
}

/*============================================================================
 *                               get_timestamp
 *============================================================================*/
uint64_t KeyValueStore::get_timestamp()
{
    struct timeval tv;
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
        printf("Error: %s does not exist\n", ROOT_DIR);
        exit(EXIT_FAILURE);
    }

    // need at least these bytes (e.g. to fully decode a <k,v,t> read from disk:
    // <keylen, valuelen, key, value, timestamp>)
    assert(MERGE_BUFSIZE >= (2 * sizeof(uint64_t) + 2 * MAX_KVTSIZE + sizeof(uint64_t)));
}
