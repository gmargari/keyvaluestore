#include "Global.h"
#include "KVTDiskFile.h"

/*========================================================================
 *                               KVTDiskFile
 *========================================================================*/
KVTDiskFile::KVTDiskFile()
{
    m_vfile = new VFile(true);
}

/*========================================================================
 *                              ~KVTDiskFile
 *========================================================================*/
KVTDiskFile::~KVTDiskFile()
{
    delete m_vfile;
}

/*=======================================================================*
 *                                 open
 *=======================================================================*/
bool KVTDiskFile::open(char *filename)
{
    return m_vfile->fs_open(filename);
}

/*=======================================================================*
 *                             open_existing
 *=======================================================================*/
bool KVTDiskFile::open_existing(char *filename)
{
    return m_vfile->fs_open_existing(filename);
}

/*=======================================================================*
 *                              open_unique
 *=======================================================================*/
bool KVTDiskFile::open_unique()
{
    return m_vfile->fs_open_unique();
}

/*=======================================================================*
 *                            delete_from_disk
 *=======================================================================*/
void KVTDiskFile::delete_from_disk()
{
    m_vfile->fs_delete();
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