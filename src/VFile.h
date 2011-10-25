#ifndef FILESIM_H
#define FILESIM_H

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
    // fs_read() and fs_write() are guaranteed to read or write 'count'
    // bytes. that is, they will repeatedly call internal functions until
    // they read or write all 'count' bytes.
    ssize_t         fs_read(char *buf, size_t count);
    ssize_t         fs_write(const char *buf, size_t count);
    off_t           fs_seek(off_t offset, int whence);
    off_t           fs_tell();
    void            fs_rewind();
    // NOTE: truncate() system call does not change file offset. our
    // implementation sets offset to file size after truncating.
    void            fs_truncate(off_t length);
    void            fs_delete();
    char           *fs_name();
    uint64_t        fs_size();
    void            fs_sync();

protected:

    char           *m_basefilename;
    uint64_t        m_size;
    off_t           m_offset;

    vector<char *>  m_names;     // we simulate a single virtual large file_
    vector<int>     m_filedescs; // _using many physical files.
    int             m_cur;       // where we are in 'm_filedescs' and 'm_names'

    // in case of real IO, the Filesim simulates a single virtual large file
    // using many physical files (of max size MAX_FILE_SIZE). the following
    // functions set or get information about the current physical file used
    bool            add_new_physical_file(bool open_existing);
    uint64_t        cur_fs_size();
    off_t           cur_fs_seek(off_t offs, int whence);
    off_t           cur_fs_tell();
    off_t           cur_fs_rewind();
    ssize_t         cur_fs_write(const char *buf, size_t count);
    ssize_t         cur_fs_read(char *buf, size_t count);
};

#endif
