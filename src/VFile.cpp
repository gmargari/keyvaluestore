#include "Global.h"
#include "VFile.h"

#include "Statistics.h"
#include "Buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

/*============================================================================
 *                                 VFile
 *============================================================================*/
VFile::VFile()
    : m_simmode(SIMMODE_REAL_IO), m_basefilename(NULL), m_size(0), m_offset(0),
      m_names(), m_filedescs(), m_cur(-1)
{
    assert(sizeof(off_t) == 8); // ensure 64bit offsets (TODO: move it elsewhere)
}

/*============================================================================
 *                                ~VFile
 *============================================================================*/
VFile::~VFile()
{
    fs_close();

    for (int i = 0; i < (int)m_names.size(); i++) {
        free(m_names[i]);
        m_names[i] = NULL;
    }

    free(m_basefilename);
}

/*============================================================================
 *                         add_new_physical_file
 *============================================================================*/
bool VFile::add_new_physical_file(bool open_existing)
{
    char newfname[100], *fname;
    int fd, fdflag;

    sprintf(newfname, "%s%s%02d", m_basefilename, VFILE_PART_PREFIX, (int)m_filedescs.size());
    fname = strdup(newfname);
    assert(fname);

    if (DBGLVL > 0) {
        printf("[DEBUG]  %s [+]\n", fname);
    }

    fdflag = O_RDWR;
    if (open_existing == false) {
        fdflag |= O_CREAT | O_TRUNC;
    }

    if ((fd = open(fname, fdflag, S_IRUSR | S_IWUSR)) == -1) {
        printf("Error: add_new_physical_file: open('%s')\n", fname);
        perror("");
        exit(EXIT_FAILURE);
    }

    m_names.push_back(fname);
    m_filedescs.push_back(fd);
    m_cur = m_filedescs.size() - 1;

    return true;
}

/*============================================================================
 *                               fs_open_new
 *============================================================================*/
bool VFile::fs_open_new(char *filename)
{
    m_basefilename = strdup(filename);
    return add_new_physical_file(false);
}

/*============================================================================
 *                             fs_open_existing
 *============================================================================*/
bool VFile::fs_open_existing(char *filename)
{
    char fname[100], vfilename[100];
    FILE *fp;
    struct stat filestatus;
    long int filesize;

    m_basefilename = strdup(filename);

    sprintf(fname, "%s%s", m_basefilename, VFILE_INFO_SUFFIX);
    if ((fp = fopen(fname, "r")) == NULL) {
        printf("Error: fs_open_existing: fopen('%s')\n", fname);
        perror("");
        exit(EXIT_FAILURE);
    }

    while(fscanf(fp, "%s %ld\n", vfilename, &filesize) == 2) {

        // check file size of file to be opened is the one expected.
        stat(vfilename, &filestatus);
        if (filestatus.st_size != filesize) {
            printf("Error: File [%s] should have size %ld but has %d\n", vfilename, filesize, filestatus.st_size);
            exit(EXIT_FAILURE);
        }

        // the only thing we actually need from info file is the number of
        // vfile files. if there are 3 files, then these should be named as
        // m_basefilename.VFILE_PART_PREFIX.[00|01|02].
        // the first time add_new_physical_file() is called, it will try to
        // open m_basefilename.VFILE_PART_PREFIX.00. the next time, XXX.01,
        // next time XXX.02 etc.
        add_new_physical_file(true);
        m_size += filesize;

        // check file opened from add_new_physical_file() is the one expected.
        if (strcmp(m_names.back(), vfilename) != 0) {
            printf("Error: File opened should be [%s] but is [%s]\n", vfilename, m_names.back());
            exit(EXIT_FAILURE);
        }
    }

    fclose(fp);

    return true;
}

/*============================================================================
 *                                  close
 *============================================================================*/
void VFile::fs_close()
{
    char fname[100];
    FILE *fp;
    struct stat filestatus;

    fs_sync();

    // save VFile metadata to disk and close all physical files of this VFile
    if (m_filedescs.size()) {
        sprintf(fname, "%s%s", m_basefilename, VFILE_INFO_SUFFIX);
        if ((fp = fopen(fname, "w")) == NULL) {
            printf("Error: save_info: fopen('%s')\n", fname);
            perror("");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < (int)m_filedescs.size(); i++) {
            stat(m_names[i], &filestatus);
            fprintf(fp, "%s %ld\n", m_names[i], filestatus.st_size);
            if (close(m_filedescs[i]) == -1) {
                printf("Error: fs_close(): close('%s')\n", m_names[i]);
                perror("");
                exit(EXIT_FAILURE);
            }
        }

        fclose(fp);
    }

    m_offset = 0;
}

/*============================================================================
 *                                 fs_read
 *============================================================================*/
ssize_t VFile::fs_read(Buffer *buf, size_t count)
{
    size_t bytes_read, num;

    for (bytes_read = 0; bytes_read < count; ) {
        if ((num = cur_fs_read(buf->m_buf + buf->m_bytes_in_buf + bytes_read, count - bytes_read)) == 0) {
            break; // no bytes left in file
        }
        bytes_read += num;
    }

    return bytes_read;
}

/*============================================================================
 *                                fs_write
 *============================================================================*/
ssize_t VFile::fs_write(Buffer *buf)
{
    return fs_write(buf, buf->m_bytes_in_buf);
}

/*============================================================================
 *                                fs_write
 *============================================================================*/
ssize_t VFile::fs_write(Buffer *buf, size_t count)
{
    size_t bytes_written, num;

    for (bytes_written = 0; bytes_written < count; ) {
        num = cur_fs_write(buf->m_buf + bytes_written, count - bytes_written);
        bytes_written += num;
    }

    assert(bytes_written == count);

    return bytes_written;
}

/*============================================================================
 *                                 fs_seek
 *============================================================================*/
off_t VFile::fs_seek(off_t offs, int whence)
{
    off_t offs_in_file;
    int fileno;

    if (whence == SEEK_SET) {
        m_offset = offs;
    } else if (whence == SEEK_CUR) {
        m_offset += offs;
    } else if (whence == SEEK_END) {
        m_offset = m_size + offs;
    }

    if (m_simmode == SIMMODE_REAL_IO) {

        /*
        * if seek is out of current file limits, check if there are previous
        * or next files in m_filedescs and if we can get to that file. if yes,
        * advance to that file.
        */

        // m_offset is the absolute offset inside the virtual file
        if (m_offset > (off_t)m_size) {
            printf("Haven't yet implemented seeking beyond end of file\n");
            exit(EXIT_FAILURE);
        }

        // calculate the file and the offset within the file
        fileno = m_offset / MAX_FILE_SIZE;
        offs_in_file = m_offset % MAX_FILE_SIZE;

        // example: if maxfilesize = 10bytes, and we:
        //    1) create a file
        //    2) write 10 bytes in file
        // then offset = 10, size = 10. In this case, fileno will be '1'
        // and offset will be '0', meaning that we have to seek to 0 byte of
        // file 1. Instead we'll seek at fileno 0 at position 10.
        if (fileno > (int)m_names.size() - 1) {
            fileno--;
            offs_in_file = MAX_FILE_SIZE;
        }

        // advance to that file
        m_cur = fileno;

        // seek within that file
        if (lseek(m_filedescs[m_cur], offs_in_file, SEEK_SET) == (off_t)-1) {
            printf("Error: fs_seek(): lseek('%s', %Ld, SEEK_SET)\n", m_names[m_cur], offs_in_file);
            perror("");
            exit(EXIT_FAILURE);
        }
    }

    return m_offset;
}

/*============================================================================
 *                                  fs_tell
 *============================================================================*/
off_t VFile::fs_tell()
{
    return m_offset;
}

/*============================================================================
 *                                 fs_rewind
 *============================================================================*/
void VFile::fs_rewind()
{
    fs_seek(0, SEEK_SET);
}

/*============================================================================
 *                                fs_truncate
 *============================================================================*/
void VFile::fs_truncate(off_t length)
{
    off_t len;
    int new_files, num;

    if (m_simmode == SIMMODE_REAL_IO) {

        new_files = (int) ceil((float)length / (float)MAX_FILE_SIZE);

        /*
         * if we need to shrink the virtual file
         */
        if (new_files < (int)m_names.size()) {

            // if length == 0 then new_files = 0. keep first file and truncate to 0
            if (new_files == 0) {
                new_files = 1;
            }

            // delete all files except the first 'new_files'
            assert(m_filedescs.size() == m_names.size());
            for (int i = new_files; i < (int)m_names.size(); i++) {

                if (DBGLVL > 0) {
                    printf("[DEBUG]  %s [-]\n", m_names[i]);
                }

                if (remove(m_names[i]) == -1) {
                    printf("Error: fs_truncate(): remove('%s')\n", m_names[i]);
                    perror("");
                    exit(EXIT_FAILURE);
                }
                free(m_names[i]);
            }

            m_names.erase(m_names.begin() + new_files, m_names.end());
            m_filedescs.erase(m_filedescs.begin() + new_files, m_filedescs.end());
        }
        /*
         * else, if we need to extend the virtual file
         */
        else if (new_files > (int)m_names.size()) {

            // create 'new_files - m_names.size()' new files.
            num = m_names.size();
            for (int i = 0; i < (int)(new_files - num); i++) {
                add_new_physical_file(false);
            }

            // set the size of the first 'new_files - 1' files to MAX_FILE_SIZE
            assert((int)m_names.size() == new_files);
            for (int i = 0; i < new_files - 1; i++) {
                if (ftruncate64(m_filedescs[i], MAX_FILE_SIZE) == (off_t)-1) {
                    printf("Error: fs_truncate(): ftruncate64('%s', %Ld) \n", m_names[i], MAX_FILE_SIZE);
                    perror("");
                    exit(EXIT_FAILURE);
                }
            }
        }

        m_cur = new_files - 1;

        if (length && length % MAX_FILE_SIZE != 0) {
            len = length % MAX_FILE_SIZE;
        } else if (length ) {
            len = MAX_FILE_SIZE;
        } else {
            len = length;
        }

        if (ftruncate64(m_filedescs[m_cur], len) == (off_t)-1) {
            printf("Error: fs_truncate(): ftruncate64('%s', %Ld) \n", m_names[m_cur], len);
            perror("");
            exit(EXIT_FAILURE);
        }
    }

    m_size = length;

    // NOTE: truncate system call does not change file offset. our
    // implementation sets offset to file size after truncating
    fs_seek(m_size, SEEK_SET);
}

/*============================================================================
 *                               fs_delete
 *============================================================================*/
void VFile::fs_delete() // TODO: rename this function to fs_rm (public) and create a protected fs_unlink() that will be called also from fs_truncate.
{
    if (m_simmode == SIMMODE_REAL_IO) {
        assert(m_names.size() == m_filedescs.size());
        for (int i = 0; i < (int)m_names.size(); i++) {
            if (DBGLVL > 0) {
                printf("[DEBUG]  %s [-]\n", m_names[i]);
            }

            if (remove(m_names[i]) == -1) {
                printf("Error: fs_delete(): remove('%s')\n", m_names[i]);
                perror("");
                exit(EXIT_FAILURE);
            }
            free(m_names[i]);
        }

        m_names.erase(m_names.begin(), m_names.end());
        m_filedescs.erase(m_filedescs.begin(), m_filedescs.end());
    }
}

/*============================================================================
 *                                 fs_name
 *============================================================================*/
char *VFile::fs_name()
{
    return m_basefilename;
}

/*============================================================================
 *                                 fs_size
 *============================================================================*/
uint64_t VFile::fs_size()
{
    return m_size;
}

/*============================================================================
 *                                 fs_sync
 *============================================================================*/
void VFile::fs_sync()
{
    if (m_simmode == SIMMODE_REAL_IO) {
        for (int i = 0; i < (int)m_filedescs.size(); i++) {
            time_start(&(g_stats.write_time));
//             fsync(m_filedescs[i]);
            time_end(&(g_stats.write_time));
        }
    }
}

/*============================================================================
 *                               cur_fs_read
 *============================================================================*/
ssize_t VFile::cur_fs_read(char *buf, size_t count)
{
    ssize_t actually_read;

    // TODO: remove/reduce calls to cur_fs_size/cur_fs_tell, they are syscalls

    assert(m_cur != -1);

    if (m_simmode == SIMMODE_REAL_IO) {

        // if we reached end of vfile, then no bytes left to read
        if (fs_tell() == (off_t)fs_size()) {
            return 0;
        }
        assert(fs_tell() <= (off_t)fs_size());

        // if we reached end of current physical file, check if there's a next
        // file in m_filedescs vector. if yes, advance to it. if no, error
        if (cur_fs_tell() == (off_t)cur_fs_size()) {
            if (cur_fs_size() == MAX_FILE_SIZE && m_cur < (int)m_filedescs.size() - 1) {
                m_cur++;
                cur_fs_rewind();
            } else {
                assert("shouldn't get here" && 0);
            }
        }

        // check if we're going to cross max file size limit: if yes, we're going
        // to read as many bytes as we can from current file and return the
        // number of bytes read. caller must call again fs_read() to
        // read the remaining bytes.
        if (cur_fs_tell() + count > MAX_FILE_SIZE) {
            count = MAX_FILE_SIZE - cur_fs_tell();
        }

        time_start(&(g_stats.read_time));
        if ((actually_read = read(m_filedescs[m_cur], buf, count)) == -1) {
            printf("Error: cur_fs_read(): read('%s', %d)\n", m_names[m_cur], count);
            perror("");
            exit(EXIT_FAILURE);
        }
        time_end(&(g_stats.read_time));
    } else {
        actually_read = count;
    }

    m_offset += actually_read;

    // update global statistics
    bytes_inc(&g_stats.bytes_read, actually_read);
    num_inc(&(g_stats.num_reads), 1);

    assert(actually_read);
    return actually_read;
}

/*============================================================================
 *                                cur_fs_write
 *============================================================================*/
ssize_t VFile::cur_fs_write(const char *buf, size_t count)
{
    ssize_t actually_written;

    if (m_simmode == SIMMODE_REAL_IO) {

        assert(m_cur != -1);

        // if we reached end of current file, check if there's a next file in
        // m_filedescs vector. if yes, advance to it. if no, create a new file.
        if (cur_fs_tell() == MAX_FILE_SIZE) {
            if (m_cur == (int)m_filedescs.size() - 1) {
                add_new_physical_file(false);
            } else {
                m_cur++;
                cur_fs_rewind();
            }
        }

        // (similar to cur_fs_read)
        if (cur_fs_tell() + count > MAX_FILE_SIZE) {
            count = MAX_FILE_SIZE - cur_fs_tell();
        }

        time_start(&(g_stats.write_time));
        if ((actually_written = write(m_filedescs[m_cur], buf, count)) == -1) {
            printf("Error: cur_fs_write(): write('%s', %d)\n", m_names[m_cur], count);
            perror("");
            exit(EXIT_FAILURE);
        }
        time_end(&(g_stats.write_time));

    } else {
        actually_written = count;
    }

    m_offset += actually_written;
    if (m_offset > (off_t)m_size) {
        m_size = m_offset;
    }

    // update global statistics
    bytes_inc(&g_stats.bytes_written, actually_written);
    num_inc(&(g_stats.num_writes), 1);

    return actually_written;
}

/*============================================================================
 *                                 cur_fs_size
 *============================================================================*/
uint64_t VFile::cur_fs_size()
{
    struct stat filestatus;

    assert(m_simmode == SIMMODE_REAL_IO);
    assert(m_cur != -1);

    stat(m_names[m_cur], &filestatus);
    return filestatus.st_size;
}

/*============================================================================
 *                                cur_fs_seek
 *============================================================================*/
off_t VFile::cur_fs_seek(off_t offs, int whence)
{
    off_t newoffs;

    assert(m_simmode == SIMMODE_REAL_IO);
    assert(m_cur != -1);

    if ((newoffs = lseek(m_filedescs[m_cur], offs, whence)) == (off_t)-1) {
        printf("Error: cur_fs_seek(): lseek('%s', %Ld, %d)\n", m_names[m_cur], offs, whence);
        perror("");
        exit(EXIT_FAILURE);
    }

    return newoffs;
}

/*============================================================================
 *                                cur_fs_tell
 *============================================================================*/
off_t VFile::cur_fs_tell()
{
    return cur_fs_seek(0, SEEK_CUR);
}

/*============================================================================
 *                                cur_fs_tell
 *============================================================================*/
off_t VFile::cur_fs_rewind()
{
    return cur_fs_seek(0, SEEK_SET);
}
