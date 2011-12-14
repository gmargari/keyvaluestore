// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_DISKSTORE_H_
#define SRC_DISKSTORE_H_

#include <stdint.h>
#include <pthread.h>
#include <vector>

#include "./Global.h"

using std::vector;

class DiskFile;
class DiskFileInputStream;

class DiskStore {
  public:
    friend class NomergeCompactionManager;
    friend class GeomCompactionManager;
    friend class RangemergeCompactionManager;
    friend class CassandraCompactionManager;
    friend class Scanner;

    /**
     * constructor
     */
    DiskStore();

    /**
     * destructor
     */
    ~DiskStore();

    /**
     * number of <key, value> pairs in diskstore
     *
     * @return number of <key, value> pairs in diskstore
     */
    uint64_t get_num_keys();

    /**
     * byte size of diskstore
     *
     * @return byte size of <key, value> pairs in diskstore
     */
    uint64_t get_size();

    /**
     * return number of disk files
     */
    int get_num_disk_files();

    /**
     * diskstore contains a read-write lock: scanners read-lock diskstore
     * and compaction manager write-locks diskstore, to avoid reading and
     * modifying 'm_disk_files' concurrently
     */
    void read_lock();
    void read_unlock();
    void write_lock();
    void write_unlock();

  private:
    bool load_from_disk();
    bool save_to_disk();

    vector<DiskFile *> m_disk_files;
    pthread_rwlock_t   m_rwlock;
};

#endif  // SRC_DISKSTORE_H_

/*============================================================================
 *                                 read_lock
 *============================================================================*/
inline void DiskStore::read_lock() {
    pthread_rwlock_rdlock(&m_rwlock);
}

/*============================================================================
 *                                read_unlock
 *============================================================================*/
inline void DiskStore::read_unlock() {
    pthread_rwlock_unlock(&m_rwlock);
}

/*============================================================================
 *                                 write_lock
 *============================================================================*/
inline void DiskStore::write_lock() {
    pthread_rwlock_wrlock(&m_rwlock);
}

/*============================================================================
 *                                write_unlock
 *============================================================================*/
inline void DiskStore::write_unlock() {
    pthread_rwlock_unlock(&m_rwlock);
}
