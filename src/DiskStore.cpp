#include "Global.h"
#include "DiskStore.h"

#include "DiskFile.h"
#include "DiskFileInputStream.h"

#include <cstdlib>
#include <cassert>
#include <cstring>

#include <cstdio>

using namespace std;

/*============================================================================
 *                                 DiskStore
 *============================================================================*/
DiskStore::DiskStore()
    : m_disk_files(), m_disk_istreams()
{
    char fname[100];
    FILE *fp;
    int num_dfiles, max_dfiles_num;

    // open existing diskstore, if any
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
    } else {
        DiskFile::set_max_dfile_num(0);
    }
}

/*============================================================================
 *                                ~DiskStore
 *============================================================================*/
DiskStore::~DiskStore()
{
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

    for (int i = 0; i < (int)m_disk_files.size(); i++) {
        delete m_disk_files[i];
    }
    m_disk_files.clear();
}

/*============================================================================
 *                               get_num_keys
 *============================================================================*/
uint64_t DiskStore::get_num_keys()
{
    uint64_t total_keys = 0;

    for (int i = 0; i < (int)m_disk_files.size(); i++) {
        total_keys += m_disk_files[i]->get_num_keys();
    }

    return total_keys;
}

/*============================================================================
 *                                get_size
 *============================================================================*/
uint64_t DiskStore::get_size()
{
    uint64_t total_size = 0;

    for (int i = 0; i < (int)m_disk_files.size(); i++) {
        total_size += m_disk_files[i]->get_size();
    }

    return total_size;
}

/*============================================================================
 *                            get_num_disk_files
 *============================================================================*/
int DiskStore::get_num_disk_files()
{
    return m_disk_files.size();
}
