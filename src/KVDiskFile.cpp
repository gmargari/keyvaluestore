#include "Global.h"
#include "KVDiskFile.h"

/*========================================================================
 *                               KVDiskFile
 *========================================================================*/
KVDiskFile::KVDiskFile()
{
    m_vfile = new VFile(true);
}

/*========================================================================
 *                              ~KVDiskFile
 *========================================================================*/
KVDiskFile::~KVDiskFile()
{
    delete m_vfile;
}

/*=======================================================================*
 *                                 open
 *=======================================================================*/
bool KVDiskFile::open(char *filename)
{
    return m_vfile->fs_open(filename);
}

/*=======================================================================*
 *                             open_existing
 *=======================================================================*/
bool KVDiskFile::open_existing(char *filename)
{
    return m_vfile->fs_open_existing(filename);
}

/*=======================================================================*
 *                              open_unique
 *=======================================================================*/
bool KVDiskFile::open_unique()
{
    return m_vfile->fs_open_unique();
}

/*=======================================================================*
 *                            delete_from_disk
 *=======================================================================*/
void KVDiskFile::delete_from_disk()
{
    m_vfile->fs_delete();
}

/*=======================================================================*
 *                              sanity_check
 *=======================================================================*/
void KVDiskFile::sanity_check()
{
    return_if_dbglvl_lt_2();
}

// char *KVDiskFile::name()     { return m_vfile->fs_name(); }
// void KVDiskFile::rewind()    { m_vfile->fs_rewind(); }
// void KVDiskFile::sync()      { m_vfile->fs_sync(); }
// void KVDiskFile::close()     { m_vfile->fs_close(); }