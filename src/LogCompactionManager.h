// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_LOGCOMPACTIONMANAGER_H_
#define SRC_LOGCOMPACTIONMANAGER_H_

#include "./GeomCompactionManager.h"

class LogCompactionManager: public GeomCompactionManager {
  public:
    /**
     * constructor
     */
    LogCompactionManager(MemStore *memstore, DiskStore *diskstore)
        : GeomCompactionManager(memstore, diskstore) {
        GeomCompactionManager::set_R(2);
    }

    /**
     * destructor
     */
    ~LogCompactionManager() { }
};

#endif  // SRC_LOGCOMPACTIONMANAGER_H_
