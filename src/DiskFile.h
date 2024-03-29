// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_DISKFILE_H_
#define SRC_DISKFILE_H_

#include <stdint.h>
#include <sys/types.h>

#include "./Global.h"
#include "./leveldb/slice.h"

class VFile;
class VFileIndex;
class Buffer;

class DiskFile {
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
    void get_first_last_term(Slice *first, Slice *last);

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
     * append all buffer bytes to this diskfile, clear buffer
     */
    uint32_t append(Buffer *buf);

    /**
     * sync pending diskfile writes to disk
     */
    void sync();

    /**
     * find the location on disk where the value corresponding to key 'term'
     * might have been stored. normally, after this call we would read all
     * bytes stored on diskfile between 'start_off' and 'end_off' and linearly
     * search for a <'term', value> pair.
     *
     * @param key (in) key to search for
     * @param start_off (out) start offset on disk file
     * @param end_off (out) end offset on disk file
     */
    bool search(Slice key, off_t *start_off, off_t *end_off);

    /**
     * set the index for this diskfile
     */
    void set_file_index(VFileIndex  *index);

    /**
     * set the number of keys stored on this diskfile
     */
    void set_num_keys(uint64_t num_keys);

    // Undefined methods (just remove Weffc++ warning)
    DiskFile(const DiskFile&);
    DiskFile& operator=(const DiskFile&);

  private:
    VFile       *m_file;           // file in which we store KVs
    VFileIndex  *m_index;          // index of <key,offs> for diskfile
    static int   m_max_dfile_num;  // create unique filenames
    bool         m_deleted;        // whether this file was delete from disk
};

#endif  // SRC_DISKFILE_H_
