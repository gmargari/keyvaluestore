#include "Global.h"
#include "KVTDiskFile.h"
#include "VFile.h"
#include "VFileIndex.h"

/*========================================================================
 *                               KVTDiskFile
 *========================================================================*/
KVTDiskFile::KVTDiskFile()
{
    m_vfile = new VFile();
    m_vfile_index = new VFileIndex();
    m_vfile_numkeys = 0;
}

/*========================================================================
 *                              ~KVTDiskFile
 *========================================================================*/
KVTDiskFile::~KVTDiskFile()
{
    delete m_vfile;
    delete m_vfile_index;
}

/*=======================================================================*
 *                                 open
 *=======================================================================*/
bool KVTDiskFile::open(char *filename)
{
    m_vfile_numkeys = 0;
    return m_vfile->fs_open(filename);
}

/*=======================================================================*
 *                             open_existing
 *=======================================================================*/
bool KVTDiskFile::open_existing(char *filename)
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

/*=======================================================================*
 *                              open_unique
 *=======================================================================*/
bool KVTDiskFile::open_unique()
{
    static int n = 0;
    static char filename[100];

    m_vfile_numkeys = 0;
    sprintf(filename, "%s%s%04d", TMPFILEDIR, TMPFILEPREFIX, n++);
    if (m_vfile->fs_open(filename)) {
        return true;
    } else {
        return false;
    }
}

/*=======================================================================*
 *                            delete_from_disk
 *=======================================================================*/
void KVTDiskFile::delete_from_disk()
{
    m_vfile->fs_close();
    m_vfile->fs_delete();
}

/*=======================================================================*
 *                              get_num_keys
 *=======================================================================*/
uint64_t KVTDiskFile::get_num_keys()
{
    return m_vfile_numkeys;
}

/*=======================================================================*
 *                                get_size
 *=======================================================================*/
uint64_t KVTDiskFile::get_size()
{
    return m_vfile->fs_size();
}

/*========================================================================
 *                         get_first_last_term
 *========================================================================*/
void KVTDiskFile::get_first_last_term(const char **first, const char **last)
{
    m_vfile_index->get_first_last_term(first, last);
}

/*=======================================================================*
 *                              sanity_check
 *=======================================================================*/
int KVTDiskFile::sanity_check()
{
    return 1;
}
