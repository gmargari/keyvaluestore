#include "Global.h"
#include "DiskFile.h"

#include "VFile.h"
#include "VFileIndex.h"
#include "Buffer.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int DiskFile::m_max_dfile_num = 0;

/*============================================================================
 *                                 DiskFile
 *============================================================================*/
DiskFile::DiskFile()
    : m_file(), m_index(), m_stored_keys(0), m_deleted(false)
{
    m_file = new VFile();
    m_index = new VFileIndex();
}

/*============================================================================
 *                                ~DiskFile
 *============================================================================*/
DiskFile::~DiskFile()
{
    if (m_deleted == false) {
        char fname[1000];
        int fd;

        sprintf(fname, "%s%s", m_file->fs_name(), VFILE_INDEX_SUFFIX);
        if ((fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) == -1) {
            perror(fname);
            exit(EXIT_FAILURE);
        }
        m_index->save_to_disk(fd);
        close(fd);
    }

    delete m_file;
    delete m_index;
}

/*============================================================================
 *                               open_existing
 *============================================================================*/
bool DiskFile::open_existing(char *filename)
{
    if (m_file->fs_open_existing(filename)) {
        char fname[1000];
        int fd;

        sprintf(fname, "%s%s", filename, VFILE_INDEX_SUFFIX);
        if ((fd = open(fname, O_RDONLY)) == -1) {
            perror(fname);
            exit(EXIT_FAILURE);
        }
        m_index->load_from_disk(fd);
        close(fd);

        // TODO
        // m_stored_keys =
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

    m_stored_keys = 0;
    sprintf(filename, "%s%s%04d", ROOT_DIR, DISKFILE_PREFIX, m_max_dfile_num++);
    if (m_file->fs_open_new(filename)) {
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
    char filename[1000];

    sprintf(filename, "%s%s", m_file->fs_name(), VFILE_INDEX_SUFFIX);
    remove(filename);

    m_file->fs_close();
    m_file->fs_delete();
    m_deleted = true;
}

/*============================================================================
 *                                get_num_keys
 *============================================================================*/
uint64_t DiskFile::get_num_keys()
{
    return m_stored_keys;
}

/*============================================================================
 *                                  get_size
 *============================================================================*/
uint64_t DiskFile::get_size()
{
    return m_file->fs_size();
}

/*============================================================================
 *                            get_first_last_term
 *============================================================================*/
void DiskFile::get_first_last_term(const char **first, const char **last)
{
    m_index->get_first_last_term(first, last);
}

/*============================================================================
 *                                  get_name
 *============================================================================*/
char *DiskFile::get_name()
{
    return m_file->fs_name();
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
    return buf->fill(m_file, bytes, offs);
}

/*============================================================================
 *                                   fill
 *============================================================================*/
uint32_t DiskFile::fill(Buffer *buf, off_t offs)
{
    return buf->fill(m_file, offs);
}

/*============================================================================
 *                                  append
 *============================================================================*/
uint32_t DiskFile::append(Buffer *buf)
{
    return buf->append(m_file);
}

/*============================================================================
 *                                  sync
 *============================================================================*/
void DiskFile::sync()
{
    m_file->fs_sync();
}

/*============================================================================
 *                                  search
 *============================================================================*/
bool DiskFile::search(const char *term, off_t *start_off, off_t *end_off)
{
    return m_index->search(term, start_off, end_off);
}

/*============================================================================
 *                              set_file_index
 *============================================================================*/
void DiskFile::set_file_index(VFileIndex  *index)
{
    delete m_index;
    m_index = index;
}

/*============================================================================
 *                               set_num_keys
 *============================================================================*/
void DiskFile::set_num_keys(uint64_t num_keys)
{
    m_stored_keys = num_keys;
}
