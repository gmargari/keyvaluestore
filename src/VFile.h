#ifndef FILESIM_H
#define FILESIM_H

#include <stdint.h>
#include <vector>
#include <string>
#include <sys/time.h>

class Simulator;

using std::vector;

class VFile
{
public:

    VFile(bool real_io);
    ~VFile();

    bool     fs_open(char *filename, bool open_existing = false);
    bool     fs_open_existing(char *filename);
    bool     fs_open_unique(void);
    void     fs_close(void);
    // fs_read() and fs_write() are guaranteed to read or write 'count'
    // bytes. that is, they will repeatedly call internal functions until
    // they read or write all 'count' bytes.
    ssize_t  fs_read(void *buf, size_t count);
    ssize_t  fs_write(const void *buf, size_t count);
    off_t    fs_seek(off_t offset, int whence);
    off_t    fs_tell(void);
    void     fs_rewind(void);
    // NOTE: truncate() system call does not change file offset. our
    // implementation sets offset to file size after truncating.
    void     fs_truncate(off_t length);
    void     fs_delete(void);
    char    *fs_name(void);
    uint64_t fs_size(void);
    void     fs_sync(void);
    uint64_t fs_append(VFile *fsrc, void *buf, size_t bufsize);

protected:

    Simulator *Sim;

    /*
    *  variables used for IO simulation
    */
    uint64_t Size;
    uint64_t Offset;
    bool     Opened;

    /*
    *  variables used for real IO
    */
    vector<char *> Names;  // we simulate a single virtual large file_
    vector<int> Filedescs; // _using many physical files.
    int Cur;                    // where we are in Filedescs and Names vecs

    // in case of real IO, the Filesim simulates a single virtual large file
    // using many physical files (of max size MAX_FILE_SIZE). the following
    // functions set or get information about the current physical file used
    uint64_t cur_fs_size(void);
    off_t    cur_fs_seek(off_t offs, int whence);
    off_t    cur_fs_tell(void);
    off_t    cur_fs_rewind(void);
    ssize_t  cur_fs_write(const void *buf, size_t count);
    ssize_t  cur_fs_read(void *buf, size_t count);
    int      sanity_check();
    /*
    *  private variables and functions used for timing parts of code
    */
    struct timeval starttime;
    struct timeval endtime;
    void start_timing(void);
    void end_timing(void);
};

#endif
