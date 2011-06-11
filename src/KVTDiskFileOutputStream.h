#ifndef KVTDISKFILEOUTPUTSTREAM_H
#define KVTDISKFILEOUTPUTSTREAM_H

#include "Global.h"
#include "KVTOutputStream.h"
#include "KVTDiskFile.h"

class KVTDiskFileOutputStream: public KVTOutputStream {

public:

    /**
     * constructor
     */
    KVTDiskFileOutputStream(KVTDiskFile *file);

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
     * flush output stream buffer
     */
    void flush();

protected:

    void sanity_check();

    VFile       *m_vfile;         // pointer to vfile
    VFileIndex  *m_vfile_index;   // pointer to vfile index
    uint32_t     m_vfile_size;    // size of vfile on disk

    char       	*m_buf;           // buffer used to serialize entries
    uint32_t     m_buf_size;      // buffer size
    uint32_t     m_bytes_in_buf;  // total bytes in byffer
    uint32_t     m_bytes_used;    // bytes already used from buffer

    off_t        m_lastoffs;      // last offset inserted in index
};

#endif
