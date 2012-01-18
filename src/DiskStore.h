// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_DISKSTORE_H_
#define SRC_DISKSTORE_H_

#include <vector>

#include "./Global.h"
#include "./DiskFile.h"
#include "./ReadWriteMutex.h"

using std::vector;

class DiskFileInputStream;

class DiskStore {
  public:
    /**
     * constructor
     */
    DiskStore();

    /**
     * destructor
     */
    ~DiskStore();

    /**
     * number of KVs in diskstore
     *
     * @return number of KVs in diskstore
     */
    uint64_t get_num_keys();

    /**
     * byte size of diskstore
     *
     * @return byte size of KVs in diskstore
     */
    uint64_t get_size();

    /**
     * add, delete or get a specific disk file to/from diskstore
     */
     void      add_diskfile(DiskFile *df, int pos);
     void      rm_diskfile(int pos);
     DiskFile *get_diskfile(int pos);
     int       get_num_disk_files();

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
    ReadWriteMutex     m_rwlock;
};

#endif  // SRC_DISKSTORE_H_

/*============================================================================
 *                               add_diskfile
 *============================================================================*/
inline void DiskStore::add_diskfile(DiskFile *dfile, int pos) {
    m_disk_files.insert(m_disk_files.begin() + pos, dfile);
}

/*============================================================================
 *                               rm_diskfile
 *============================================================================*/
inline void DiskStore::rm_diskfile(int pos) {
    delete m_disk_files[pos];
    m_disk_files.erase(m_disk_files.begin() + pos);
}

/*============================================================================
 *                               get_diskfile
 *============================================================================*/
inline DiskFile *DiskStore::get_diskfile(int pos) {
    return m_disk_files[pos];
}

/*============================================================================
 *                            get_num_disk_files
 *============================================================================*/
inline int DiskStore::get_num_disk_files() {
    return m_disk_files.size();
}

/*============================================================================
 *                                 read_lock
 *============================================================================*/
inline void DiskStore::read_lock() {
    m_rwlock.read_lock();
}

/*============================================================================
 *                                read_unlock
 *============================================================================*/
inline void DiskStore::read_unlock() {
    m_rwlock.read_unlock();
}

/*============================================================================
 *                                 write_lock
 *============================================================================*/
inline void DiskStore::write_lock() {
    m_rwlock.write_lock();
}

/*============================================================================
 *                                write_unlock
 *============================================================================*/
inline void DiskStore::write_unlock() {
    m_rwlock.write_unlock();
}
