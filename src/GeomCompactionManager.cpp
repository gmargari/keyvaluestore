// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#include "./Global.h"
#include "./GeomCompactionManager.h"

#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <vector>

#include "./DiskStore.h"
#include "./MapInputStream.h"
#include "./DiskFile.h"
#include "./DiskFileInputStream.h"
#include "./Streams.h"

uint64_t dbg_lastsize = 0;  // used for sanity_check()

/*============================================================================
 *                             GeomCompactionManager
 *============================================================================*/
GeomCompactionManager::GeomCompactionManager(MemStore *memstore,
                                             DiskStore *diskstore)
    : CompactionManager(memstore, diskstore),
      m_R(DEFAULT_GEOM_R), m_P(DEFAULT_GEOM_P), m_partition_size() {
    load_state_from_disk();
}

/*============================================================================
 *                            ~GeomCompactionManager
 *============================================================================*/
GeomCompactionManager::~GeomCompactionManager() {
    save_state_to_disk();
}

/*============================================================================
 *                                   set_R
 *============================================================================*/
void GeomCompactionManager::set_R(int r) {
    assert(r >= 2 || (m_P && r >= 1));
    m_R = r;
}

/*============================================================================
 *                                   get_R
 *============================================================================*/
int GeomCompactionManager::get_R() {
    return m_R;
}

/*============================================================================
 *                                   set_P
 *============================================================================*/
void GeomCompactionManager::set_P(int p) {
    assert(p >= 1);
    m_P = p;
}

/*============================================================================
 *                                   get_P
 *============================================================================*/
int GeomCompactionManager::get_P() {
    return m_P;
}

/*============================================================================
 *                             partition_minsize
 *============================================================================*/
int GeomCompactionManager::partition_minsize(int part_num) {
    assert(m_R >= 2 || (m_P && m_R >= 1));
    if (m_P) {
        return (int)pow(m_R, part_num);
    } else {
        return (int)pow(m_R, part_num);
    }
}

/*============================================================================
 *                             partition_maxsize
 *============================================================================*/
int GeomCompactionManager::partition_maxsize(int part_num) {
    // the only difference between P being and not being constant
    // is that in the first case partition 0 has limit R maxsize in the
    // second case it has maxsize R-1.
    assert(m_R >= 2 || (m_P && m_R >= 1));
    if (m_P) {
        return (int)pow(m_R, part_num + 1);
    } else {
        return (m_R - 1) * (int)pow(m_R, part_num);
    }
}

/*============================================================================
 *                              partition_num
 *============================================================================*/
int GeomCompactionManager::partition_num(int partition_size) {
    for (int i = 0; ; i++) {
        if (partition_size <= partition_maxsize(i)) {
            assert(partition_size >= partition_minsize(i));
            return i;
        }
    }

    assert("can't get here" && 0);
}


/*============================================================================
 *                             compute_current_R
 *============================================================================*/
int GeomCompactionManager::compute_current_R() {
    int size_of_bytes_inserted = 0;

    size_of_bytes_inserted = 1;  // 1 for memstore that will be flushed to disk
    for (int i = 0; i < (int)m_partition_size.size(); i++) {
        size_of_bytes_inserted += m_partition_size[i];
    }
    return (int)ceil( pow(size_of_bytes_inserted, 1.0 / m_P) );
}

/*============================================================================
 *                                do_flush
 *============================================================================*/
void GeomCompactionManager::do_flush() {
    DiskFile *disk_file, *memstore_file;
    vector<InputStream *> istreams;
    int size, count, part_num;

    assert(sanity_check());

    // if we bound num of partitions, re-adjust R so num of partitions is <= P
    if (m_P) {
        set_R(compute_current_R());
    }

    //--------------------------------------------------------------------------
    // select disk files to merge with memstore, add their input streams
    // to vector of streams to merge
    //--------------------------------------------------------------------------
    // merge the memstore with the sub-index in the first partition.
    // if the merged sub-index is greater than the max size of the partition,
    // merge it with the next biggest sub-index. repeat until the merged
    // sub-index fits within a partition (we may need to create a new partition
    // that will store the sub-index, if new sub-index doesn't fit in any
    // existing partition)
    count = 0;
    size = 1;  // 1 for memstore
    for (unsigned int i = 0; i < m_partition_size.size(); i++) {
        if (m_partition_size[i]) {
            count++;
            size += m_partition_size[i];
        }
        if (size <= partition_maxsize(i)) {
            break;
        }
    }
    assert(count <= m_diskstore->get_num_disk_files());

    for (int i = 0; i < count; i++) {
        DiskFile *dfile = m_diskstore->get_diskfile(i);
        istreams.push_back(new DiskFileInputStream(dfile));
    }

    //--------------------------------------------------------------------------
    // add memstore stream to vector of streams to merge, clear memstore
    //--------------------------------------------------------------------------
    memstore_file = memstore_flush_to_diskfile();
    istreams.push_back(new DiskFileInputStream(memstore_file));

    //--------------------------------------------------------------------------
    // merge streams creating a new disk file
    //--------------------------------------------------------------------------
    if (istreams.size() > 1) {
        disk_file = Streams::merge_streams(istreams);
    } else {
        disk_file = memstore_file;
    }
    for (int i = 0; i < (int)istreams.size(); i++) {
        delete istreams[i];
    }

    //--------------------------------------------------------------------------
    // delete merged files
    //--------------------------------------------------------------------------
    if (istreams.size() > 1) {
        memstore_file->delete_from_disk();
        delete memstore_file;
    }

    m_diskstore->write_lock();
    for (int i = count - 1; i >= 0; i--) {
        m_diskstore->get_diskfile(i)->delete_from_disk();
        m_diskstore->rm_diskfile(i);
    }

    //--------------------------------------------------------------------------
    // add new file to DiskStore (add first: it contains the most recent KVs)
    //--------------------------------------------------------------------------
    m_diskstore->add_diskfile(disk_file, 0);
    m_diskstore->write_unlock();

    //--------------------------------------------------------------------------
    // update vector of partitions
    //--------------------------------------------------------------------------
    // zero the sizes of the 'count' partitions merged
    for (int i = 0; i < (int)m_partition_size.size(); i++) {
        if (m_partition_size[i] && count > 0) {
            m_partition_size[i] = 0;
            count--;
        }
    }

    // add new partition
    part_num = partition_num(size);
    if (part_num == (int)m_partition_size.size()) {
        m_partition_size.push_back(size);
    } else {
        assert(part_num < (int)m_partition_size.size());
        m_partition_size[part_num] = size;
    }

    assert(sanity_check());
}

/*============================================================================
 *                              save_state_to_disk
 *============================================================================*/
bool GeomCompactionManager::save_state_to_disk() {
    char fname[100];
    FILE *fp;

    if (m_partition_size.size()) {
        sprintf(fname, "%s%s", ROOT_DIR, CMMANAGER_FILENAME);
        if ((fp = fopen(fname, "w")) == NULL) {
            printf("Error: fopen('%s')\n", fname);
            perror("");
            exit(EXIT_FAILURE);
        }

        fprintf(fp, "cmmanager: %s\n", "geometric");
        fprintf(fp, "R: %d\n", m_R);
        fprintf(fp, "P: %d\n", m_P);
        fprintf(fp, "partitions: %d\n", (int)m_partition_size.size());
        for (int i = 0; i < (int)m_partition_size.size(); i++) {
            fprintf(fp, "partition_size: %d\n", m_partition_size[i]);
        }

        fclose(fp);

        return true;
    }

    return false;
}

/*============================================================================
 *                            load_state_from_disk
 *============================================================================*/
bool GeomCompactionManager::load_state_from_disk() {
    char fname[100], cmmanager[100];
    FILE *fp;
    int num_partitions, part_size;

    // open existing diskstore, if any
    sprintf(fname, "%s%s", ROOT_DIR, CMMANAGER_FILENAME);
    if ((fp = fopen(fname, "r")) != NULL) {
        fscanf(fp, "cmmanager: %s\n", cmmanager);
        if (strcmp(cmmanager, "geometric") != 0) {
            printf("Error: expected 'geometric' cmanager in %s, found '%s'\n",
                   fname, cmmanager);
            exit(EXIT_FAILURE);
        }
        fscanf(fp, "R: %d\n", &m_R);
        fscanf(fp, "P: %d\n", &m_P);
        fscanf(fp, "partitions: %d\n", &num_partitions);
        for (int i = 0; i < num_partitions; i++) {
            fscanf(fp, "partition_size: %d\n", &part_size);
            m_partition_size.push_back(part_size);
            dbg_lastsize += part_size;
        }

        assert((int)m_partition_size.size() == num_partitions);
        sanity_check();

        fclose(fp);

        return true;
    }

    return false;
}

/*============================================================================
 *                                sanity_check
 *============================================================================*/
int GeomCompactionManager::sanity_check() {
    uint64_t cursize = 0;

    assert((int)m_partition_size.size() >= m_diskstore->get_num_disk_files());
    for (int i = 0; i < (int)m_partition_size.size(); i++) {
        // if P != 0, there's a case when R increases before a flush and
        // the assertion below does not hold
        assert(m_P != 0 || (m_partition_size[i] == 0 ||
                 m_partition_size[i] >= partition_minsize(i)));
        assert(m_partition_size[i] <= partition_maxsize(i));
        cursize += m_partition_size[i];
    }
    assert(cursize == dbg_lastsize || cursize == dbg_lastsize + 1);
    dbg_lastsize = cursize;

    if (m_P) {
        assert(m_diskstore->get_num_disk_files() <= m_P);
    }

    return 1;
}
