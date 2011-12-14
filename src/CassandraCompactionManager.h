// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_CASSANDRACOMPACTIONMANAGER_H_
#define SRC_CASSANDRACOMPACTIONMANAGER_H_

#include <vector>

#include "./Global.h"
#include "./CompactionManager.h"

using std::vector;

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

    int sanity_check();

    int         m_L;            // max number of files per level
    vector<int> m_level_files;  // number of existing files per level
};

#endif  // SRC_CASSANDRACOMPACTIONMANAGER_H_
