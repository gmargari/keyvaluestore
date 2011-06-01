#ifndef KVDISKFILESCANNER_H
#define KVDISKFILESCANNER_H

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
     * get next <key, value> pair
     */
    bool read(const char **key, const char **value);
    
protected:
    
    KVDiskFile *m_kvdiskfile;
    char       *m_buf;
    uint32_t    m_buf_size;
    uint32_t    m_bytes_in_buf;
    uint32_t    m_bytes_used;  
};

#endif
