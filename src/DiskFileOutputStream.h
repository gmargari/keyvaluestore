#ifndef KVTDISKFILEOUTPUTSTREAM_H
#define KVTDISKFILEOUTPUTSTREAM_H

#include "OutputStream.h"

#include <sys/types.h>

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
    void reset();
    bool write(const char *key, size_t keylen, const char *value, size_t valuelen, uint64_t timestamp);
    void flush();

protected:

    DiskFile    *m_diskfile;
    off_t        m_offs;          // offset within diskfile we currently are
    uint64_t     m_vfile_size;    // size of vfile
    Buffer      *m_buf;           // buffer used for I/O
    char        *m_last_key;      // last key written to disk
    off_t        m_last_offs;     // offset of last key written to disk
    off_t        m_last_idx_offs; // last offset inserted in index
};

#endif
