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

  protected:
    bool load_from_disk();
    bool save_to_disk();

    vector<DiskFile *> m_disk_files;
    pthread_rwlock_t   m_rwlock;
};

#endif  // SRC_DISKSTORE_H_
