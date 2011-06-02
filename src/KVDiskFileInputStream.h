#ifndef KVDISKFILEINPUTSTREAM_H
#define KVDISKFILEINPUTSTREAM_H

#include "Global.h"
#include "KVInputStream.h"
#include "KVDiskFile.h"

class KVDiskFileInputStream: public KVInputStream {

public:

    /**
     * constructor
     */
    KVDiskFileInputStream(KVDiskFile *file);

    /**
     * destructor
     */
    ~KVDiskFileInputStream();

    // inherited from KVInputStream
    /**
     * reset scanner
     */
    void reset();
    
    /**
     * get next <key, value> pair from stream
     *
     * @return false if no <k,v> pair left. else, true.
     */
    bool read(const char **key, const char **value);
    
protected:
    
    void sanity_check();

    KVDiskFile *m_kvdiskfile;
    char       *m_buf;
    uint32_t    m_buf_size;
    uint32_t    m_bytes_in_buf;
    uint32_t    m_bytes_used;
};

#endif
