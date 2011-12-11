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

#define my_perror_exit() STMT ( printf("Error: %s => %s:%d\n", __FUNCTION__, __FILE__, __LINE__); perror(""); exit(EXIT_FAILURE); )
#define my_exit()        STMT ( printf("Error: %s => %s:%d\n", __FUNCTION__, __FILE__, __LINE__); exit(EXIT_FAILURE); )

/*============================================================================
 *                                 VFile
 *============================================================================*/
VFile::VFile()
    : m_basefilename(NULL), m_names(), m_filedescs() {
    assert(sizeof(off_t) == 8); // ensure 64bit offsets
}

/*============================================================================
 *                                ~VFile
 *============================================================================*/
VFile::~VFile() {
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
bool VFile::add_new_physical_file(bool open_existing) {
    char newfname[100], *fname;
    int fd, fdflag;

    sprintf(newfname, "%s%s%02d", m_basefilename, VFILE_PART_PREFIX, (int)m_filedescs.size());
    fname = strdup(newfname);
    assert(fname);

    if (DBGLVL > 0) {
        printf("# [DEBUG]   %s [+]\n", fname);
    }

    fdflag = O_RDWR | O_APPEND;
    if (open_existing == false) {
        fdflag |= O_CREAT | O_TRUNC;
    }

    if ((fd = open(fname, fdflag, S_IRUSR | S_IWUSR)) == -1) {
        my_perror_exit();
    }

    m_names.push_back(fname);
    m_filedescs.push_back(fd);

    return true;
}

/*============================================================================
 *                               fs_open_new
 *============================================================================*/
bool VFile::fs_open_new(char *filename) {
    m_basefilename = strdup(filename);
    return add_new_physical_file(false);
}

/*============================================================================
 *                             fs_open_existing
 *============================================================================*/
bool VFile::fs_open_existing(char *filename) {
    char fname[100], vfilename[100];
    FILE *fp;
    struct stat filestatus;
    long int filesize;

    m_basefilename = strdup(filename);

    sprintf(fname, "%s%s", m_basefilename, VFILE_INFO_SUFFIX);
    if ((fp = fopen(fname, "r")) == NULL) {
        my_perror_exit();
    }

    while(fscanf(fp, "%s %ld\n", vfilename, &filesize) == 2) {

        // check file size of file to be opened is the one expected.
        stat(vfilename, &filestatus);
        if (filestatus.st_size != filesize) {
            my_exit();
        }

        add_new_physical_file(true);

        // check file opened from add_new_physical_file() is the one expected
        if (strcmp(m_names.back(), vfilename) != 0) {
            my_exit();
        }
    }

    fclose(fp);

    return true;
}

/*============================================================================
 *                                  close
 *============================================================================*/
void VFile::fs_close() {
    char fname[100];
    FILE *fp;
    struct stat filestatus;

    fs_sync();

    // save VFile metadata to disk and close all physical files of this VFile
    if (m_filedescs.size()) {
        sprintf(fname, "%s%s", m_basefilename, VFILE_INFO_SUFFIX);
        if ((fp = fopen(fname, "w")) == NULL) {
            my_perror_exit();
        }

        for (int i = 0; i < (int)m_filedescs.size(); i++) {
            stat(m_names[i], &filestatus);
            fprintf(fp, "%s %ld\n", m_names[i], filestatus.st_size);
            if (close(m_filedescs[i]) == -1) {
                my_perror_exit();
            }
        }

        fclose(fp);
    }
}

/*============================================================================
 *                                 fs_pread
 *============================================================================*/
ssize_t VFile::fs_pread(char *buf, size_t count, off_t offs) {
    size_t bytes_read, num;

    for (bytes_read = 0; bytes_read < count; ) {
        if ((num = cur_fs_pread(buf + bytes_read, count - bytes_read, offs + bytes_read)) == 0) {
            break; // no bytes left in file
        }
        bytes_read += num;
    }

    return bytes_read;
}

/*============================================================================
 *                                fs_append
 *============================================================================*/
ssize_t VFile::fs_append(const char *buf, size_t count) {
    size_t bytes_written;

    for (bytes_written = 0; bytes_written < count; ) {
        bytes_written += cur_fs_append(buf + bytes_written, count - bytes_written);
    }

    assert(bytes_written == count);

    return bytes_written;
}

/*============================================================================
 *                                fs_truncate
 *============================================================================*/
void VFile::fs_truncate(off_t length) {
    off_t len;
    int new_files, num;

    new_files = (int) ceil((float)length / (float)MAX_FILE_SIZE);

    // if we need to shrink the virtual file
    if (new_files < (int)m_names.size()) {

        // if length == 0 then new_files = 0. keep first file and truncate to 0
        if (new_files == 0) {
            new_files = 1;
        }

        // delete all files except the first 'new_files'
        assert(m_filedescs.size() == m_names.size());
        for (int i = new_files; i < (int)m_names.size(); i++) {

            if (DBGLVL > 0) {
                printf("# [DEBUG]   %s [-]\n", m_names[i]);
            }

            if (remove(m_names[i]) == -1) {
                my_perror_exit();
            }
            free(m_names[i]);
        }

        m_names.erase(m_names.begin() + new_files, m_names.end());
        m_filedescs.erase(m_filedescs.begin() + new_files, m_filedescs.end());
    }
    // else, if we need to extend the virtual file
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
                my_perror_exit();
            }
        }
    }

    if (length && length % MAX_FILE_SIZE != 0) {
        len = length % MAX_FILE_SIZE;
    } else if (length ) {
        len = MAX_FILE_SIZE;
    } else {
        len = length;
    }

    if (ftruncate64(m_filedescs[new_files - 1], len) == (off_t)-1) {
        my_perror_exit();
    }
}

/*============================================================================
 *                               fs_delete
 *============================================================================*/
void VFile::fs_delete() {
    char fname[100];

    sprintf(fname, "%s%s", m_basefilename, VFILE_INFO_SUFFIX);
    remove(fname);

    assert(m_names.size() == m_filedescs.size());
    for (int i = 0; i < (int)m_names.size(); i++) {
        if (DBGLVL > 0) {
            printf("# [DEBUG]   %s [-]\n", m_names[i]);
        }

        if (remove(m_names[i]) == -1) {
            my_perror_exit();
        }
        free(m_names[i]);
    }

    m_names.erase(m_names.begin(), m_names.end());
    m_filedescs.erase(m_filedescs.begin(), m_filedescs.end());
}

/*============================================================================
 *                                 fs_name
 *============================================================================*/
char *VFile::fs_name() {
    return m_basefilename;
}

/*============================================================================
 *                                 fs_size
 *============================================================================*/
uint64_t VFile::fs_size() {
    uint64_t size = 0;
    struct stat filestatus;

    for (int i = 0; i < (int)m_filedescs.size(); i++) {
        stat(m_names[i], &filestatus);
        size += filestatus.st_size;
    }

    return size;
}

/*============================================================================
 *                                 fs_sync
 *============================================================================*/
void VFile::fs_sync() {
    for (int i = 0; i < (int)m_filedescs.size(); i++) {
        time_start(&(g_stats.write_time));
        fsync(m_filedescs[i]);
        time_end(&(g_stats.write_time));
    }
}

/*============================================================================
 *                               cur_fs_pread
 *============================================================================*/
ssize_t VFile::cur_fs_pread(char *buf, size_t count, off_t offs) {
    ssize_t actually_read;
    off_t offs_in_file;
    int fileno;

    if (offs >= (off_t)fs_size()) { // if no bytes left to read
        return 0;
    }

    // calculate the file and the offset within the file
    fileno = offs / MAX_FILE_SIZE;
    offs_in_file = offs % MAX_FILE_SIZE;

    time_start(&(g_stats.read_time));
    if ((actually_read = pread(m_filedescs[fileno], buf, count, offs_in_file)) == -1) {
        my_perror_exit();
    }
    time_end(&(g_stats.read_time));

    // update global statistics
    bytes_inc(&g_stats.bytes_read, actually_read);
    num_inc(&(g_stats.num_reads), 1);

    return actually_read;
}

/*============================================================================
 *                                cur_fs_append
 *============================================================================*/
ssize_t VFile::cur_fs_append(const char *buf, size_t count) {
    ssize_t actually_written;
    size_t bytes_left;
    int fileno;
    struct stat filestatus;

    // how many bytes can we write in last file
    fileno = m_filedescs.size() - 1;
    stat(m_names[fileno], &filestatus);
    bytes_left = MAX_FILE_SIZE - filestatus.st_size;
    if (bytes_left == 0) {
        add_new_physical_file(false);
        fileno++;
        bytes_left = MAX_FILE_SIZE;
    }

    // write as many bytes as we can in current file
    if (count > bytes_left) {
        count = bytes_left;
    }

    time_start(&(g_stats.write_time));
    if ((actually_written = pwrite(m_filedescs[fileno], buf, count, 0)) == -1) {
        my_perror_exit();
    }
    time_end(&(g_stats.write_time));

    // update global statistics
    bytes_inc(&g_stats.bytes_written, actually_written);
    num_inc(&(g_stats.num_writes), 1);

    return actually_written;
}
