#ifndef CASSANDRA_COMPACTIONMANAGER_H
#define CASSANDRA_COMPACTIONMANAGER_H

#include "CompactionManager.h"

#include <stdint.h>
#include <vector>

using std::vector;

class MemStore;
class DiskStore;

class CassandraCompactionManager: public CompactionManager {

public:

    /**
     * constructor
     */
    CassandraCompactionManager(MemStore *memstore, DiskStore *diskstore);

    /**
     * destructor
     */
    ~CassandraCompactionManager();

    // inherited from CompactionManager (see CompactionManager.h for description)
    void flush_bytes();

    /**
     * set/get value of parameter L
     */
    void set_L(int l);
    int  get_L();

protected:

    /**
     * save/load compaction manager state to/from disk
     */
    bool save_state_to_disk();
    bool load_state_from_disk();

    void print_partitions();

    int sanity_check();

    int         m_L;            // max number of files per level
    vector<int> m_level_files;  // number of existing files per level

};

#endif
