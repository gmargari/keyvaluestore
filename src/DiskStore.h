#ifndef DISKSTORE_H
#define DISKSTORE_H

#include <stdint.h>
#include <vector>

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
friend class RangeScanner;

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

    vector<DiskFile *> m_disk_files;
};

#endif
