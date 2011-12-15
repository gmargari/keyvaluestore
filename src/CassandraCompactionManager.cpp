// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#include "./CassandraCompactionManager.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <vector>

#include "./DiskStore.h"
#include "./DiskFile.h"
#include "./DiskFileInputStream.h"
#include "./Streams.h"

/*============================================================================
 *                          CassandraCompactionManager
 *============================================================================*/
CassandraCompactionManager::CassandraCompactionManager(MemStore *memstore, DiskStore *diskstore)
    : CompactionManager(memstore, diskstore),
      m_L(DEFAULT_CASS_K), m_level_files() {
    load_state_from_disk();
}

/*============================================================================
 *                         ~CassandraCompactionManager
 *============================================================================*/
CassandraCompactionManager::~CassandraCompactionManager() {
    save_state_to_disk();
}

/*============================================================================
 *                                   set_L
 *============================================================================*/
void CassandraCompactionManager::set_L(int l) {
    m_L = l;
}

/*============================================================================
 *                                   get_L
 *============================================================================*/
int CassandraCompactionManager::get_L() {
    return m_L;
}

/*============================================================================
 *                                flush_bytes
 *============================================================================*/
void CassandraCompactionManager::flush_bytes() {
    DiskFile *disk_file, *memstore_file;
    DiskFileInputStream *istream;
    vector<InputStream *> istreams_to_merge;
    vector<DiskFile *> &r_disk_files = m_diskstore->m_disk_files;
    unsigned int merge_lvl;

    assert(sanity_check());

    //--------------------------------------------------------------------------
    // 1) check if we need to perform compactions before flushing memstore
    //--------------------------------------------------------------------------
    for (merge_lvl = 0; merge_lvl < m_level_files.size(); merge_lvl++) {
        if (m_level_files[merge_lvl] < m_L) {
            break;
        }
    }

    //--------------------------------------------------------------------------
    // 2) perform a cascade of compactions, if neccessary
    //--------------------------------------------------------------------------
    // if merge_lvl != 0, then for each level L from 0 to 'merge_lvl-1' we need
    // to merge its files, producing a new file at level L+1
    for (int lvl = merge_lvl-1; lvl >= 0; lvl--) {

        //----------------------------------------------------------------------
        // select disk files to merge
        //----------------------------------------------------------------------
        istreams_to_merge.clear();
        for (int i = lvl*m_L; i < lvl*m_L + m_L; i++) {
            istream = new DiskFileInputStream(r_disk_files[i], MERGE_BUFSIZE);
            istreams_to_merge.push_back(istream);
        }

        //----------------------------------------------------------------------
        // merge streams creating a new disk file
        //----------------------------------------------------------------------
        disk_file = Streams::merge_streams(istreams_to_merge);

        //----------------------------------------------------------------------
        // delete merged files and streams
        //----------------------------------------------------------------------
        for (int i = 0; i < (int)istreams_to_merge.size(); i++) {
            delete istreams_to_merge[i];
        }

        pthread_rwlock_wrlock(&m_diskstore->m_rwlock);
        for (int i = lvl*m_L; i < lvl*m_L + m_L; i++) {
            r_disk_files[i]->delete_from_disk();
            delete r_disk_files[i];
        }
        r_disk_files.erase(r_disk_files.begin() + lvl*m_L, r_disk_files.begin() + lvl*m_L + m_L);

        //----------------------------------------------------------------------
        // add new file to DiskStore
        //----------------------------------------------------------------------
        r_disk_files.insert(r_disk_files.begin() + lvl*m_L, disk_file);
        pthread_rwlock_unlock(&m_diskstore->m_rwlock);

        //----------------------------------------------------------------------
        // update vector 'm_level_files'
        //----------------------------------------------------------------------
        m_level_files[lvl] = 0;
        if (lvl + 1 == (int)m_level_files.size()) {
            m_level_files.push_back(0);
        }
        m_level_files[lvl + 1] += 1;
    }

    //--------------------------------------------------------------------------
    // 3) flush memstore to disk, at level 0
    //--------------------------------------------------------------------------
    memstore_file = memstore_flush_to_diskfile();
    memstore_clear();

    pthread_rwlock_wrlock(&m_diskstore->m_rwlock);
    // insert first in diskstore as it contains the most recent <k,v> pairs
    m_diskstore->m_disk_files.insert(m_diskstore->m_disk_files.begin(), memstore_file);
    if (m_level_files.size() == 0) {
        m_level_files.push_back(0);
    }
    m_level_files[0] += 1;
    pthread_rwlock_unlock(&m_diskstore->m_rwlock);

    assert(sanity_check());
}

/*============================================================================
 *                              save_state_to_disk
 *============================================================================*/
bool CassandraCompactionManager::save_state_to_disk() {
    char fname[100];
    FILE *fp;

    sprintf(fname, "%s%s", ROOT_DIR, CMMANAGER_FILENAME);
    if ((fp = fopen(fname, "w")) == NULL) {
        printf("Error: fopen('%s')\n", fname);
        perror("");
        exit(EXIT_FAILURE);
    }

    fprintf(fp, "cmmanager: %s\n", "cassandra");
    fprintf(fp, "L: %d\n", m_L);
    fprintf(fp, "levels: %d\n", m_level_files.size());
    for (int i = 0; i < (int)m_level_files.size(); i++) {
        fprintf(fp, "level_files: %d\n", m_level_files[i]);
    }

    fclose(fp);

    return true;
}

/*============================================================================
 *                            load_state_from_disk
 *============================================================================*/
bool CassandraCompactionManager::load_state_from_disk() {
    char fname[100], cmmanager[100];
    FILE *fp;
    int num_levels, level_files;

    // open existing diskstore, if any
    sprintf(fname, "%s%s", ROOT_DIR, CMMANAGER_FILENAME);
    if ((fp = fopen(fname, "r")) != NULL) {
        fscanf(fp, "cmmanager: %s\n", cmmanager);
        if (strcmp(cmmanager, "cassandra") != 0) {
            printf("Error: expected 'cassandra' cmanager in file %s, found '%s'\n", fname, cmmanager);
            exit(EXIT_FAILURE);
        }
        fscanf(fp, "L: %d\n", &m_L);
        fscanf(fp, "levels: %d\n", &num_levels);
        for (int i = 0; i < num_levels; i++) {
            fscanf(fp, "level_files: %d\n", &level_files);
            m_level_files.push_back(level_files);
        }

        assert((int)m_level_files.size() == num_levels);
        fclose(fp);

        sanity_check();

        return true;
    }

    return false;
}

/*============================================================================
 *                                sanity_check
 *============================================================================*/
int CassandraCompactionManager::sanity_check() {
    int size = 0;
    static int last_size = 0;

    assert(m_level_files.size() == 0 || m_level_files[0] >= 1);
    for (uint i = 0; i < m_level_files.size(); i++) {
        assert(m_level_files[i] >= 0 && m_level_files[i] <= m_L);
        size += m_level_files[i] * pow(m_L, i);
    }

    if (last_size != 0) {
        // sanity_check is called both when entering and exiting flush_bytes
        assert(last_size == size || last_size + 1 == size);
    }
    last_size = size;

    return 1;
}
