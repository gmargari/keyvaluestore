// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#include "./Global.h"
#include "./KeyValueStore.h"

#include <assert.h>
#include <sys/stat.h>
#include <iostream>

#include "./MemStore.h"
#include "./DiskStore.h"
#include "./NomergeCompactionManager.h"
#include "./ImmCompactionManager.h"
#include "./LogCompactionManager.h"
#include "./GeomCompactionManager.h"
#include "./RangemergeCompactionManager.h"
#include "./CassandraCompactionManager.h"
#include "./Statistics.h"
#include "./leveldb/slice.h"

using std::cout;
using std::cerr;
using std::endl;

/*============================================================================
 *                               KeyValueStore
 *============================================================================*/
KeyValueStore::KeyValueStore(cm_type type)
    : m_memstore(), m_diskstore(), m_compactionmanager() {
    m_memstore = new MemStore();
    m_diskstore = new DiskStore();
    switch (type) {
        case KeyValueStore::NOMERGE_CM:
            m_compactionmanager = new NomergeCompactionManager(m_memstore,
                                                               m_diskstore);
            break;
        case KeyValueStore::IMM_CM:
            m_compactionmanager = new ImmCompactionManager(m_memstore,
                                                           m_diskstore);
            break;
        case KeyValueStore::GEOM_CM:
            m_compactionmanager = new GeomCompactionManager(m_memstore,
                                                            m_diskstore);
            break;
        case KeyValueStore::LOG_CM:
            m_compactionmanager = new LogCompactionManager(m_memstore,
                                                           m_diskstore);
            break;
        case KeyValueStore::RNGMERGE_CM:
            m_compactionmanager = new RangemergeCompactionManager(m_memstore,
                                                                  m_diskstore);
            break;
        case KeyValueStore::CASSANDRA_CM:
            m_compactionmanager = new CassandraCompactionManager(m_memstore,
                                                                 m_diskstore);
            break;
    }
    set_memstore_maxsize(DEFAULT_MEMSTORE_SIZE);
    check_parameters();
    global_stats_init();
}

/*============================================================================
 *                              ~KeyValueStore
 *============================================================================*/
KeyValueStore::~KeyValueStore() {
    delete m_memstore;
    delete m_diskstore;
    delete m_compactionmanager;
}

/*============================================================================
 *                           set_memstore_maxsize
 *============================================================================*/
void KeyValueStore::set_memstore_maxsize(uint64_t maxsize) {
    m_memstore->set_maxsize(maxsize);
}

/*============================================================================
 *                           set_memstore_maxsize
 *============================================================================*/
uint64_t KeyValueStore::get_memstore_maxsize() {
    return m_memstore->get_maxsize();
}

/*============================================================================
 *                                   put
 *============================================================================*/
bool KeyValueStore::put(const char *key, uint32_t keylen, const char *value,
                        uint32_t valuelen, uint64_t timestamp) {
    Slice k(key, keylen), v(value, valuelen);

    assert(m_memstore->get_size() <= m_memstore->get_maxsize());
    if (m_memstore->will_fill(k, v, timestamp)) {
        time_start(&(g_stats.compaction_time));
        request_flush();
        time_end(&(g_stats.compaction_time));
        g_stats.disk_files = m_diskstore->get_num_disk_files();
    }

    return m_memstore->put(k, v, timestamp);
}

/*============================================================================
 *                                   put
 *============================================================================*/
bool KeyValueStore::put(const char *key, uint32_t keylen, const char *value,
                        uint32_t valuelen) {
    return put(key, keylen, value, valuelen, get_timestamp());
}

/*============================================================================
 *                             get_num_mem_keys
 *============================================================================*/
uint64_t KeyValueStore::get_num_mem_keys() {
    return m_memstore->get_num_keys();
}

/*============================================================================
 *                             get_num_disk_keys
 *============================================================================*/
uint64_t KeyValueStore::get_num_disk_keys() {
    return m_diskstore->get_num_keys();
}

/*============================================================================
 *                               get_mem_size
 *============================================================================*/
uint64_t KeyValueStore::get_mem_size() {
    return m_memstore->get_size();
}

/*============================================================================
 *                               get_disk_size
 *============================================================================*/
uint64_t KeyValueStore::get_disk_size() {
    return m_diskstore->get_size();
}

/*============================================================================
 *                             get_num_disk_files
 *============================================================================*/
int KeyValueStore::get_num_disk_files() {
    return m_diskstore->get_num_disk_files();
}

/*============================================================================
 *                             memstore_will_fill
 *============================================================================*/
bool KeyValueStore::memstore_will_fill(const char *key, uint32_t keylen,
                                       const char *value, uint32_t valuelen,
                                       uint64_t timestamp) {
    Slice k(key, keylen), v(value, valuelen);

    return m_memstore->will_fill(k, v, timestamp);
}

/*============================================================================
 *                             memstore_will_fill
 *============================================================================*/
bool KeyValueStore::memstore_will_fill(const char *key, uint32_t keylen,
                                       const char *value, uint32_t valuelen) {
    return memstore_will_fill(key, keylen, value, valuelen, 0);
}

/*============================================================================
 *                                do_flush
 *============================================================================*/
void KeyValueStore::request_flush() {
    m_compactionmanager->request_flush();
}

/*============================================================================
 *                          get_compaction_manager
 *============================================================================*/
CompactionManager *KeyValueStore::get_compaction_manager() {
    return m_compactionmanager;
}

/*============================================================================
 *                               get_timestamp
 *============================================================================*/
uint64_t KeyValueStore::get_timestamp() {
//    struct timeval tv;
    static uint64_t t = 0;

//     gettimeofday(&tv, NULL);
//     return (uint64_t)(tv.tv_sec*1000000 + tv.tv_usec);
    return ++t;
}

/*============================================================================
 *                            check_parameters
 *============================================================================*/
void KeyValueStore::check_parameters() {  // TODO: move elsewhere ?
    struct stat st;
    uint64_t maxsize;

    if (stat(ROOT_DIR, &st) != 0) {
        cerr << "Error: " << ROOT_DIR << " does not exist" << endl;
        exit(EXIT_FAILURE);
    }

    // need at least these bytes (e.g. to fully decode a <k,v,t> read from disk:
    // <keylen, valuelen, key, value, timestamp>)
    maxsize = 2 * sizeof(uint64_t) + MAX_KEY_SIZE + MAX_VALUE_SIZE
                + sizeof(uint64_t);
    if (MERGEBUF_SIZE < maxsize) {
        cerr << "Error: MERGEBUF_SIZE must be >= " << maxsize << endl;
        exit(EXIT_FAILURE);
    }
}
