#include "Global.h"
#include "DiskFile.h"

#include "VFile.h"
#include "VFileIndex.h"
#include "Buffer.h"

int DiskFile::m_max_dfile_num = 0;

/*============================================================================
 *                                 DiskFile
 *============================================================================*/
DiskFile::DiskFile()
    : m_vfile(), m_vfile_index(), m_vfile_numkeys(0)
{
    m_vfile = new VFile();
    m_vfile_index = new VFileIndex();
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
        // TODO: read vfile index, vfile num keys
        // m_vfile_index =
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
    char filename[100];

    m_vfile_numkeys = 0;
    sprintf(filename, "%s%s%04d", ROOT_DIR, DISKFILE_PREFIX, m_max_dfile_num++);
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
 *                                  get_name
 *============================================================================*/
char *DiskFile::get_name()
{
    return m_vfile->fs_name();
}

/*============================================================================
 *                             set_max_dfile_num
 *============================================================================*/
void DiskFile::set_max_dfile_num(int num)
{
    m_max_dfile_num = num;
}

/*============================================================================
 *                             set_max_dfile_num
 *============================================================================*/
int DiskFile::get_max_dfile_num()
{
    return m_max_dfile_num;
}

/*============================================================================
 *                                   fill
 *============================================================================*/
uint32_t DiskFile::fill(Buffer *buf, uint32_t bytes, off_t offs)
{
    return buf->fill(m_vfile, bytes, offs);
}

/*============================================================================
 *                                   fill
 *============================================================================*/
uint32_t DiskFile::fill(Buffer *buf, off_t offs)
{
    return buf->fill(m_vfile, offs);
}

/*============================================================================
 *                                  flush
 *============================================================================*/
uint32_t DiskFile::flush(Buffer *buf, off_t offs)
{
    return buf->flush(m_vfile, offs);
}

/*============================================================================
 *                                  sync
 *============================================================================*/
void DiskFile::sync()
{
    m_vfile->fs_sync();
}

/*============================================================================
 *                                  search
 *============================================================================*/
bool DiskFile::search(const char *term, off_t *start_off, off_t *end_off)
{
    return m_vfile_index->search(term, start_off, end_off);
}

/*============================================================================
 *                              set_file_index
 *============================================================================*/
void DiskFile::set_file_index(VFileIndex  *index)
{
    delete m_vfile_index;
    m_vfile_index = index;
}

/*============================================================================
 *                               set_num_keys
 *============================================================================*/
void DiskFile::set_num_keys(uint64_t num_keys)
{
    m_vfile_numkeys = num_keys;
}
