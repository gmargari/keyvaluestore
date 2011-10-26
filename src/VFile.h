#ifndef FILESIM_H
#define FILESIM_H

/**
 * VFile class simulates a single virtual large file of theoritically
 * unlimited size using many physical files, each of max size 'MAX_FILE_SIZE'
 */

#include <stdint.h>
#include <vector>
#include <string>
#include <sys/time.h>

using std::vector;

class VFile
{
public:

    /**
     * constructor
     */
    VFile();

    /**
     * destructor
     */
    ~VFile();

    bool            fs_open_new(char *filename);
    bool            fs_open_existing(char *filename);
    void            fs_close();
    ssize_t         fs_pread(char *buf, size_t count, off_t offs);
    ssize_t         fs_pwrite(const char *buf, size_t count, off_t offs);
    void            fs_truncate(off_t length);
    void            fs_delete();
    char           *fs_name();
    uint64_t        fs_size();
    void            fs_sync();

protected:

    char           *m_basefilename;
    vector<char *>  m_names;     // we simulate a single virtual large file_
    vector<int>     m_filedescs; // _using many physical files.

    bool            add_new_physical_file(bool open_existing);
    uint64_t        cur_fs_size(int fileno);
    ssize_t         cur_fs_pwrite(const char *buf, size_t count, off_t offs);
    ssize_t         cur_fs_pread(char *buf, size_t count, off_t offs);
};

#endif
