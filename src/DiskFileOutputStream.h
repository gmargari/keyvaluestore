#ifndef KVTDISKFILEOUTPUTSTREAM_H
#define KVTDISKFILEOUTPUTSTREAM_H

#include "OutputStream.h"

#include <cstdlib>

class DiskFile;
class Buffer;

class DiskFileOutputStream: public OutputStream {

public:

    /**
     * constructor
     */
    DiskFileOutputStream(DiskFile *file, uint32_t bufsize);

    /**
     * destructor
     */
    ~DiskFileOutputStream();

    // inherited from OutputStream
    /**
     * reset scanner
     */
    void reset();

    /**
     * write <key, value, timestamp> tuple to stream
     *
     * @return false if could not write tuple to stream. else, true.
     */
    bool write(const char *key, size_t keylen, const char *value, size_t valuelen, uint64_t timestamp);

    /**
     * flush output stream buffer. user must call this function after
     * writing all data in order to ensure data durability and to properly
     * create index of underlying vfile.
     */
    void flush();

protected:

    DiskFile    *m_diskfile;
    uint64_t     m_vfile_size;    // size of vfile
    Buffer      *m_buf;           // buffer used for I/O
    char        *m_last_key;      // last key written to disk
    off_t        m_last_offs;     // offset of last key written to disk
    off_t        m_last_idx_offs; // last offset inserted in index
};

#endif
