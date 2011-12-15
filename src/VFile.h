// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_VFILE_H_
#define SRC_VFILE_H_

/**
 * VFile class simulates a single virtual large file of theoritically
 * unlimited size using many physical files, each of max size 'MAX_FILE_SIZE'
 */

#include <stdint.h>
#include <vector>
#include <string>

#include "./Global.h"

using std::vector;

class VFile {
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
    ssize_t         fs_append(const char *buf, size_t count);
    void            fs_truncate(off_t length);
    void            fs_delete();
    char           *fs_name();
    uint64_t        fs_size();
    void            fs_sync();

    // Undefined methods (just remove Weffc++ warning)
    VFile(const VFile&);
    VFile& operator=(const VFile&);

  private:
    char           *m_basefilename;
    vector<char *>  m_names;      // we simulate a single virtual large file_
    vector<int>     m_filedescs;  // _using many physical files.

    bool            add_new_physical_file(bool open_existing);
    ssize_t         cur_fs_append(const char *buf, size_t count);
    ssize_t         cur_fs_pread(char *buf, size_t count, off_t offs);
};

#endif  // SRC_VFILE_H_
