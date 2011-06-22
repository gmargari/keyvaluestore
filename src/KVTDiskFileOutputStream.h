#ifndef KVTDISKFILEOUTPUTSTREAM_H
#define KVTDISKFILEOUTPUTSTREAM_H

#include "KVTOutputStream.h"

#include <cstdlib>

class KVTDiskFile;
class VFile;
class VFileIndex;

class KVTDiskFileOutputStream: public KVTOutputStream {

public:

    /**
     * constructor
     */
    KVTDiskFileOutputStream(KVTDiskFile *file, uint32_t bufsize);

    /**
     * destructor
     */
    ~KVTDiskFileOutputStream();

    // inherited from KVTOutputStream
    /**
     * reset scanner
     */
    void reset();

    /**
     * write <key, value, timestamp> tuple to stream
     *
     * @return false if could not write tuple to stream. else, true.
     */
    bool write(const char *key, const char *value, uint64_t timestamp);

    /**
     * flush output stream buffer. user must call this function after
     * writing all data in order to ensure data durability and to properly
     * create index of underlying vfile.
     */
    void flush();

protected:

    int sanity_check();

    KVTDiskFile *m_kvtdiskfile;

    char       	*m_buf;           // buffer used to serialize entries
    uint32_t     m_buf_size;      // buffer size
    uint32_t     m_bytes_in_buf;  // total bytes in byffer

    char        *m_last_key;      // last key written to disk
    off_t        m_last_offs;     // offset of last key written to disk
    off_t        m_last_idx_offs; // last offset inserted in index
};

#endif
