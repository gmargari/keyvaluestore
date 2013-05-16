// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_REMERGECOMPACTIONMANAGER_H_
#define SRC_REMERGECOMPACTIONMANAGER_H_

#include "./GeomCompactionManager.h"

class RemergeCompactionManager: public GeomCompactionManager {
  public:
    /**
     * constructor
     */
    RemergeCompactionManager(MemStore *memstore, DiskStore *diskstore)
        : GeomCompactionManager(memstore, diskstore) {
        GeomCompactionManager::set_P(1);
    }

    /**
     * destructor
     */
    ~RemergeCompactionManager() { }
};

#endif  // SRC_REMERGECOMPACTIONMANAGER_H_
