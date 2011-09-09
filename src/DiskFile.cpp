#include "Global.h"
#include "DiskFile.h"
#include "VFile.h"
#include "VFileIndex.h"

/*============================================================================
 *                                 DiskFile
 *============================================================================*/
DiskFile::DiskFile()
{
    m_vfile = new VFile();
    m_vfile_index = new VFileIndex();
    m_vfile_numkeys = 0;
}

/*============================================================================
 *                                ~DiskFile
 *============================================================================*/
DiskFile::~DiskFile()
{
    delete m_vfile;
    delete m_vfile_index;
}

/*============================================================================
 *                               open_existing
 *============================================================================*/
bool DiskFile::open_existing(char *filename)
{
    if (m_vfile->fs_open_existing(filename)) {
        // TODO: read vfile index, vfile size, vfile num keys
        // m_vfile_index =
        // m_vfile_size =
        // m_vfile_numkeys =
        return true;
    } else {
        return false;
    }
}

/*============================================================================
 *                              open_new_unique
 *============================================================================*/
bool DiskFile::open_new_unique()
{
    static int n = 0;
    static char filename[100];

    m_vfile_numkeys = 0;
    sprintf(filename, "%s%s%04d", ROOT_DIR, DISKFILE_PREFIX, n++);
    if (m_vfile->fs_open_new(filename)) {
        return true;
    } else {
        return false;
    }
}

/*============================================================================
 *                              delete_from_disk
 *============================================================================*/
void DiskFile::delete_from_disk()
{
    m_vfile->fs_close();
    m_vfile->fs_delete();
}

/*============================================================================
 *                                get_num_keys
 *============================================================================*/
uint64_t DiskFile::get_num_keys()
{
    return m_vfile_numkeys;
}

/*============================================================================
 *                                  get_size
 *============================================================================*/
uint64_t DiskFile::get_size()
{
    return m_vfile->fs_size();
}

/*============================================================================
 *                            get_first_last_term
 *============================================================================*/
void DiskFile::get_first_last_term(const char **first, const char **last)
{
    m_vfile_index->get_first_last_term(first, last);
}

/*============================================================================
 *                                sanity_check
 *============================================================================*/
int DiskFile::sanity_check()
{
    return 1;
}
