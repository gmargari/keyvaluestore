#include "Global.h"
#include "KVTDiskFile.h"
#include "VFile.h"
#include "VFileIndex.h"

/*========================================================================
 *                               KVTDiskFile
 *========================================================================*/
KVTDiskFile::KVTDiskFile()
{
    m_vfile = new VFile(true);
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
        m_vfile->fs_truncate(0); // in case file already existed
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
    m_vfile->fs_delete();
}

/*=======================================================================*
 *                                num_keys
 *=======================================================================*/
uint64_t KVTDiskFile::num_keys()
{
    return m_vfile_numkeys;
}

/*=======================================================================*
 *                                  size
 *=======================================================================*/
uint64_t KVTDiskFile::size()
{
    return m_vfile->fs_size();
}


/*=======================================================================*
 *                              sanity_check
 *=======================================================================*/
void KVTDiskFile::sanity_check()
{
    return_if_dbglvl_lt_2();
}

// char *KVTDiskFile::name()     { return m_vfile->fs_name(); }
// void KVTDiskFile::rewind()    { m_vfile->fs_rewind(); }
// void KVTDiskFile::sync()      { m_vfile->fs_sync(); }
// void KVTDiskFile::close()     { m_vfile->fs_close(); }