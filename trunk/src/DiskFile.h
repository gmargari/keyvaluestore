#ifndef KVTDISKFILE_H
#define KVTDISKFILE_H

#include <stdint.h>

class VFile;
class VFileIndex;

class DiskFile {

friend class DiskFileInputStream;
friend class DiskFileOutputStream;

public:

    /**
     * constructor
     */
    DiskFile();

    /**
     * destructor
     */
    ~DiskFile();

    /**
     * open file for read and write (create if not existing)
     *
     * @return normally, should return true
     */
    bool open(char *filename);

    /**
     * open an existing file
     *
     * @return true if file exists and was succesfully opened. otherwise false
     */
    bool open_existing(char *filename);

    /**
     * open a new unique file in /tmp/ (see TMPFILEDIR, TMPFILEPREFIX in Global.h)
     *
     * @return normally, should return true
     */
    bool open_unique();

    /**
     * delete file from disk
     */
    void delete_from_disk();

    /**
     * return number of keys (or tuples) stored on disk file
     */
    uint64_t get_num_keys();

    /**
     * return size of disk file
     */
    uint64_t get_size();

    /**
     * return first and last term stored in disk file
     */
    void get_first_last_term(const char **first, const char **last);

//     // file system name of this file
//     char *name();    // NOTE: a VFile can be composed of several disk files. modify function to return array of names?!
//     // rewind file
//     void rewind();
//     // flush all pending writes to disk
//     void sync();
//     // close file
//     void close();

protected:

    int sanity_check();

    VFile                   *m_vfile;
    VFileIndex              *m_vfile_index;
    uint64_t                 m_vfile_numkeys;
};

#endif
