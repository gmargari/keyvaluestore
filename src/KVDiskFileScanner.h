#ifndef KVDISKFILESCANNER_H
#define KVDISKFILESCANNER_H

#include "Global.h"
#include "KVScanner.h"
#include "KVDiskFile.h"

class KVDiskFileScanner: public KVScanner {

public:

    /**
     * constructor
     */
    KVDiskFileScanner(KVDiskFile *file);

    /**
     * destructor
     */
    ~KVDiskFileScanner();

    /**
     * reset scanner
     */
    void reset();
    
    /**
     * get next <key, value> pair
     */
    bool next(const char **key, const char **value);
    
protected:
    
    KVDiskFile *m_kvdiskfile;
    char       *m_buf;
    uint32_t    m_buf_size;
    uint32_t    m_bytes_in_buf;
    uint32_t    m_bytes_used;  
};

#endif
