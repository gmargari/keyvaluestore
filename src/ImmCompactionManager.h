// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_IMMCOMPACTIONMANAGER_H_
#define SRC_IMMCOMPACTIONMANAGER_H_

#include "./GeomCompactionManager.h"

class ImmCompactionManager: public GeomCompactionManager {
  public:
    /**
     * constructor
     */
    ImmCompactionManager(MemStore *memstore, DiskStore *diskstore)
        : GeomCompactionManager(memstore, diskstore) {
        GeomCompactionManager::set_P(1);
    }

    /**
     * destructor
     */
    ~ImmCompactionManager() { }
};

#endif  // SRC_IMMCOMPACTIONMANAGER_H_
