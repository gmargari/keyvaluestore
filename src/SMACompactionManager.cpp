// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#include "./Global.h"
#include "./SMACompactionManager.h"

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
#include "./Statistics.h"

/*============================================================================
 *                            SMACompactionManager
 *============================================================================*/
SMACompactionManager::SMACompactionManager(MemStore *memstore,
                                                       DiskStore *diskstore)
    : CompactionManager(memstore, diskstore),
      m_L(DEFAULT_CASS_K), m_level_files() {
    load_state_from_disk();
}

/*============================================================================
 *                           ~SMACompactionManager
 *============================================================================*/
SMACompactionManager::~SMACompactionManager() {
    save_state_to_disk();
}

/*============================================================================
 *                                   set_L
 *============================================================================*/
void SMACompactionManager::set_L(int l) {
    m_L = l;
}

/*============================================================================
 *                                   get_L
 *============================================================================*/
int SMACompactionManager::get_L() {
    return m_L;
}

/*============================================================================
 *                                do_flush
 *============================================================================*/
void SMACompactionManager::do_flush() {
    DiskFile *disk_file, *memstore_file;
    vector<InputStream *> istreams;

    assert(sanity_check());

    //--------------------------------------------------------------------------
    // flush memstore to disk, at level 0
    //--------------------------------------------------------------------------
    memstore_file = memstore_flush_to_diskfile();

    // insert first in diskstore as it contains the most recent KVs
    m_diskstore->write_lock();
    m_diskstore->add_diskfile(memstore_file, 0);
    m_diskstore->write_unlock();

    // update vector of number of files per level
    if (m_level_files.size() == 0) {
        m_level_files.push_back(0);
    }
    m_level_files[0] += 1;

    //--------------------------------------------------------------------------
    // perform a cascade of compactions, if neccessary
    //--------------------------------------------------------------------------
    // if merge_lvl != 0, then for each level L from 0 to 'merge_lvl-1' we need
    // to merge its files, producing a new file at level L+1
    for (int L = 0; L < (int)m_level_files.size(); L++) {

        if (m_level_files[L] < m_L) {
            break;
        }

        // select disk files to merge
        istreams.clear();

        int files_before = 0;
        for (int i = 0; i < L; i++) {
            files_before += m_level_files[i];
        }

        for (int i = 0; i < m_level_files[L]; i++) {
            DiskFile *dfile = m_diskstore->get_diskfile(files_before + i);
            istreams.push_back(new DiskFileInputStream(dfile));
        }

        // merge streams creating a new disk file
        disk_file = Streams::merge_streams(istreams);

        // delete streams and merged files
        for (int i = 0; i < (int)istreams.size(); i++) {
            delete istreams[i];
        }

        m_diskstore->write_lock();
        for (int i = m_level_files[L] - 1; i >= 0; i--) {
            m_diskstore->get_diskfile(files_before + i)->delete_from_disk();
            m_diskstore->rm_diskfile(files_before + i);
        }

        // add new file to DiskStore
        m_diskstore->add_diskfile(disk_file, files_before);
        m_diskstore->write_unlock();

        // update vector of number of files per level
        m_level_files[L] = 0;
        if (L + 1 == (int)m_level_files.size()) {
            m_level_files.push_back(0);
        }
        m_level_files[L + 1] += 1;
    }

    assert(sanity_check());
}

/*============================================================================
 *                              save_state_to_disk
 *============================================================================*/
bool SMACompactionManager::save_state_to_disk() {
    char fname[100];
    FILE *fp;

    sprintf(fname, "%s%s", ROOT_DIR, CMMANAGER_FILENAME);
    if ((fp = fopen(fname, "w")) == NULL) {
        printf("Error: fopen('%s')\n", fname);
        perror("");
        exit(EXIT_FAILURE);
    }

    fprintf(fp, "cmmanager: %s\n", "sma");
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
bool SMACompactionManager::load_state_from_disk() {
    char fname[100], cmmanager[100];
    FILE *fp;
    int num_levels, level_files;

    // open existing diskstore, if any
    sprintf(fname, "%s%s", ROOT_DIR, CMMANAGER_FILENAME);
    if ((fp = fopen(fname, "r")) != NULL) {
        fscanf(fp, "cmmanager: %s\n", cmmanager);
        if (strcmp(cmmanager, "sma") != 0) {
            printf("Error: expected 'sma' cmanager in %s, found '%s'\n",
                   fname, cmmanager);
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
int SMACompactionManager::sanity_check() {
    int size = 0;
    static int last_size = 0;

    for (uint i = 0; i < m_level_files.size(); i++) {
        assert(m_level_files[i] >= 0 && m_level_files[i] <= m_L);
        size += m_level_files[i] * pow(m_L, i);
    }

    if (last_size != 0) {
        // sanity_check is called both when entering and exiting do_flush
        assert(last_size == size || last_size + 1 == size);
    }
    last_size = size;

    return 1;
}
