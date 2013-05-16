// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_SMACOMPACTIONMANAGER_H_
#define SRC_SMACOMPACTIONMANAGER_H_

#include <vector>

#include "./Global.h"
#include "./CompactionManager.h"

using std::vector;

class SMACompactionManager: public CompactionManager {
  public:
    /**
     * constructor
     */
    SMACompactionManager(MemStore *memstore, DiskStore *diskstore);

    /**
     * destructor
     */
    ~SMACompactionManager();

    /**
     * set/get value of parameter L
     */
    void set_L(int l);
    int  get_L();

  private:
    // inherited from CompactionManager (see CompactionManager.h for info)
    void do_flush();

    /**
     * save/load compaction manager state to/from disk
     */
    bool save_state_to_disk();
    bool load_state_from_disk();

    int sanity_check();

    int         m_L;            // max number of files per level
    vector<int> m_level_files;  // number of existing files per level
};

#endif  // SRC_SMACOMPACTIONMANAGER_H_
