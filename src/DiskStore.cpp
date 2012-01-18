// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#include "./Global.h"
#include "./DiskStore.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "./DiskFile.h"

/*============================================================================
 *                                 DiskStore
 *============================================================================*/
DiskStore::DiskStore()
    : m_disk_files(), m_rwlock() {
    if (!load_from_disk()) {
        DiskFile::set_max_dfile_num(0);
    }
}

/*============================================================================
 *                                ~DiskStore
 *============================================================================*/
DiskStore::~DiskStore() {
    save_to_disk();

    for (int i = 0; i < (int)m_disk_files.size(); i++) {
        delete m_disk_files[i];
    }
    m_disk_files.clear();
}

/*============================================================================
 *                               get_num_keys
 *============================================================================*/
uint64_t DiskStore::get_num_keys() {
    uint64_t total_keys = 0;

    read_lock();
    for (int i = 0; i < (int)m_disk_files.size(); i++) {
        total_keys += m_disk_files[i]->get_num_keys();
    }
    read_unlock();

    return total_keys;
}

/*============================================================================
 *                                get_size
 *============================================================================*/
uint64_t DiskStore::get_size() {
    uint64_t total_size = 0;

    read_lock();
    for (int i = 0; i < (int)m_disk_files.size(); i++) {
        total_size += m_disk_files[i]->get_size();
    }
    read_unlock();

    return total_size;
}

/*============================================================================
 *                               save_to_disk
 *============================================================================*/
bool DiskStore::save_to_disk() {
    char fname[100];
    FILE *fp;

    if (m_disk_files.size()) {
        sprintf(fname, "%s%s", ROOT_DIR, DISKSTORE_FILENAME);
        if ((fp = fopen(fname, "w")) == NULL) {
            printf("Error: fopen('%s')\n", fname);
            perror("");
            exit(EXIT_FAILURE);
        }

        fprintf(fp, "diskfiles: %d\n", (int)m_disk_files.size());
        fprintf(fp, "maxdfilenum: %d\n", DiskFile::get_max_dfile_num());
        for (int i = 0; i < (int)m_disk_files.size(); i++) {
            fprintf(fp, "%s\n", m_disk_files[i]->get_name());
        }

        fclose(fp);
    }

    return true;
}

/*============================================================================
 *                              load_from_disk
 *============================================================================*/
bool DiskStore::load_from_disk() {
    char fname[100];
    FILE *fp;
    int num_dfiles, max_dfiles_num;

    sprintf(fname, "%s%s", ROOT_DIR, DISKSTORE_FILENAME);
    if ((fp = fopen(fname, "r")) != NULL) {
        fscanf(fp, "diskfiles: %d\n", &num_dfiles);
        fscanf(fp, "maxdfilenum: %d\n", &max_dfiles_num);
        for (int i = 0; i < num_dfiles; i++) {
            fscanf(fp, "%s\n", fname);
            m_disk_files.push_back(new DiskFile());
            m_disk_files.back()->open_existing(fname);
        }
        fclose(fp);

        assert(m_disk_files.size() == (unsigned)num_dfiles);

        DiskFile::set_max_dfile_num(max_dfiles_num);

        return true;
    }

    return false;
}
