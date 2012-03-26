// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_DISKFILEOUTPUTSTREAM_H_
#define SRC_DISKFILEOUTPUTSTREAM_H_

#include <sys/types.h>

#include "./Global.h"
#include "./OutputStream.h"
#include "./leveldb/slice.h"

class DiskFile;
class Buffer;
class VFileIndex;

class DiskFileOutputStream: public OutputStream {
  public:
    /**
     * constructor
     */
    DiskFileOutputStream(DiskFile *file);
    DiskFileOutputStream(DiskFile *file, uint32_t bufsize);

    /**
     * destructor
     */
    ~DiskFileOutputStream();

    // inherited from OutputStream
    bool append(Slice key, Slice value, uint64_t timestamp);
    void close();

    // Undefined methods (just remove Weffc++ warning)
    DiskFileOutputStream(const DiskFileOutputStream&);
    DiskFileOutputStream& operator=(const DiskFileOutputStream&);

  private:
    DiskFile    *m_file;           // file in which we store KVs
    uint64_t     m_file_size;      // current size of diskfile
    Buffer      *m_buf;            // buffer used for I/O

    VFileIndex  *m_index;          // index of <key,offs> for diskfile keys
    char        *m_last_key;       // last key written to disk
    uint32_t     m_last_keylen;    // length of last key written to disk
    off_t        m_last_offs;      // offset of last key written to disk
    off_t        m_last_idx_offs;  // last offset inserted in index
    uint64_t     m_stored_keys;    // number of keys stored in diskfile
};

#endif  // SRC_DISKFILEOUTPUTSTREAM_H_
