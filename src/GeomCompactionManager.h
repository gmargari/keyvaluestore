#ifndef GEOM_COMPACTIONMANAGER_H
#define GEOM_COMPACTIONMANAGER_H

#include "CompactionManager.h"

#include <vector>

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

    // inherited from CompactionManager (see CompactionManager.h for description)
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

protected:

    /**
     * return the lower bound on the size of 'num'-th partition.
     */
    int partition_minsize(int num);

    /**
     * return the upper bound on the size of 'num'-th partition
     */
    int partition_maxsize(int num);

    /**
     * given the size of a partition, return which partition is this (e.g. the 4th)
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

    int         m_R; // used to define the size limit of each partition
    int         m_P; // maximum number of partition, 0 if disabled
    vector<int> m_partition_size;

};

#endif
