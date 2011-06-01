#ifndef KVDISKFILE_H
#define KVDISKFILE_H

#include "Global.h"
#include "VFile.h"
#include "KVSerialization.h"

class KVDiskFile {

friend class KVDiskFileScanner;
    
public:

    /**
     * constructor
     */
    KVDiskFile();
    
    /**
     * destructor
     */
    ~KVDiskFile();

    /**
     * serialize and append <key, value> pair on disk file
     */
    void write(const char *key, const char *value);

//     /**
//      * read and deserialize a <key, value> pair from disk file
//      */
//     uint32_t read(const char **key, const char **value);

    /**
     * rewind file
     */
    void rewind();
    
    /**
     * flush all pending writes to disk
     */
    void sync();

    /**
     * close file
     */
    void close();

    /**
     * file system name of this file
     */
    char *name();
    
protected:
    VFile       *m_vfile;
    
    // TODO: tmp hack! -> move them to iterator!
    #define MBFUSIZE 1000
    char        m_buf[MBFUSIZE];
    uint32_t    m_bytes_in_buf;
    uint32_t    m_bytes_used;
    
};

#endif
