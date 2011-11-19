#ifndef DISKSTORE_H
#define DISKSTORE_H

#include <stdint.h>
#include <vector>
#include <pthread.h>

using std::vector;

class DiskFile;
class DiskFileInputStream;

class DiskStore {

friend class CompactionManager;
friend class NomergeCompactionManager;
friend class ImmCompactionManager;
friend class GeomCompactionManager;
friend class LogCompactionManager;
friend class RangemergeCompactionManager;
friend class CassandraCompactionManager;
friend class Scanner;

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

#endif
