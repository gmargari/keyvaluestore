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

    KVTDiskFile	*m_kvtdiskfile;
    char       	*m_buf;
    uint32_t     m_buf_size;
    uint32_t     m_bytes_in_buf;
    uint32_t     m_bytes_used;
};

#endif
