#ifndef KVTDISKFILE_H
#define KVTDISKFILE_H

#include <stdint.h>

#include <sys/types.h>

class VFile;
class VFileIndex;
class Buffer;

class DiskFile {

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
     * open an existing file
     *
     * @return true if file exists and was succesfully opened. otherwise false
     */
    bool open_existing(char *filename);

    /**
     * create a new, unique file in $ROOT_DIR (see Global.h)
     *
     * @return normally, should return true
     */
    bool open_new_unique();

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

    /**
     * name of this disk file
     */
    char *get_name();

    /**
     * set/get value of 'max_dfile_num' (set once, during initialization of
     * diskstore)
     */
    static void set_max_dfile_num(int num);
    static int  get_max_dfile_num();

    /**
     * fill free space in buffer 'buf' by reading at most 'bytes' bytes from
     * offset 'offs' of this diskfile
     *
     * @return number of bytes read
     */
    uint32_t fill(Buffer *buf, uint32_t bytes, off_t offs);

    /**
     * fill free space in buffer 'buf' by reading bytes from offset 'offs' of
     * this diskfile
     *
     * @return number of bytes read
     */
    uint32_t fill(Buffer *buf, off_t offs);

    /**
     * find the location on disk where the value corresponding to key 'term'
     * might have been stored. normally, after this call we would read all
     * bytes stored on diskfile between 'start_off' and 'end_off' and linearly
     * search for a <'term', value> pair.
     *
     * @param term (in) term to search for
     * @param start_off (out) start offset on disk file
     * @param end_off (out) end offset on disk file
     */
    bool search(const char *term, off_t *start_off, off_t *end_off);

//     // rewind file
//     void rewind();
//     // flush all pending writes to disk
//     void sync();
//     // close file
//     void close();

protected:

    VFile       *m_vfile;
    VFileIndex  *m_vfile_index;
    uint64_t     m_vfile_numkeys;
    static int   m_max_dfile_num; // create unique filenames
};

#endif
