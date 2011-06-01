#ifndef KVDISKFILEOUTPUTSTREAM_H
#define KVDISKFILEOUTPUTSTREAM_H

#include "Global.h"
#include "KVOutputStream.h"
#include "KVDiskFile.h"

class KVDiskFileOutputStream: public KVOutputStream {

public:

    /**
     * constructor
     */
    KVDiskFileOutputStream(KVDiskFile *file);

    /**
     * destructor
     */
    ~KVDiskFileOutputStream();

    // inherited from KVOutputStream
    /**
     * reset scanner
     */
    void reset();
    
    /**
     * write <key, value> to stream
     *
     * @return false if could not write <k,v> to stream. else, true.
     */
    bool write(const char *key, const char *value);

    /**
     * flush output stream buffer
     */
    void flush();
    
protected:
    
    KVDiskFile *m_kvdiskfile;
    char       *m_buf;
    uint32_t    m_buf_size;
    uint32_t    m_bytes_in_buf;
    uint32_t    m_bytes_used;  
};

#endif
