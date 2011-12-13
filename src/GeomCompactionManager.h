// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_GEOMCOMPACTIONMANAGER_H_
#define SRC_GEOMCOMPACTIONMANAGER_H_

#include <vector>

#include "./Global.h"
#include "./CompactionManager.h"

using std::vector;

class GeomCompactionManager: public CompactionManager {
  public:
    /**
     * constructor
     */
    GeomCompactionManager(MemStore *memstore, DiskStore *diskstore);

    /**
     * destructor
     */
    ~GeomCompactionManager();

    // inherited from CompactionManager (see CompactionManager.h for info)
    void flush_bytes();

    /**
     * set/get value of parameter R of Geometric Partitioning
     */
    void set_R(int r);
    int  get_R();

    /**
     * set/get value of parameter P (maximum number of disk partitions) of
     * Geometric Partitioning.
     */
    void set_P(int p);
    int  get_P();

  private:
    /**
     * return the lower bound on the size of 'num'-th partition.
     */
    int partition_minsize(int num);

    /**
     * return the upper bound on the size of 'num'-th partition
     */
    int partition_maxsize(int num);

    /**
     * given the size of a partition, return which partition is this
     */
    int partition_num(int partition_size);

    /**
     * if P is set ("bound number of disk partitions"), compute the current
     * value of R based on the number of bytes inserted in system (stored
     * in memstore & diskstore), so that the number of disk partitions is
     * bounded by P.
     * R is computed based on the total number of bytes inserted and the
     * value of P.
     */
    int compute_current_R();

    /**
     * save/load compaction manager state to/from disk
     */
    bool save_state_to_disk();
    bool load_state_from_disk();

    int sanity_check();

    int         m_R;               // size limit of each partition
    int         m_P;               // max number of partition, 0 if disabled
    vector<int> m_partition_size;  // size of each partition
};

#endif  // SRC_GEOMCOMPACTIONMANAGER_H_
