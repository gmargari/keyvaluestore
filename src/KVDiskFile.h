#ifndef KVDISKFILE_H
#define KVDISKFILE_H

#include "Global.h"
#include "VFile.h"
#include "KVSerialization.h"

class KVDiskFile {

friend class KVDiskFileInputStream;
friend class KVDiskFileOutputStream;
    
public:

    /**
     * constructor
     */
    KVDiskFile();
    
    /**
     * destructor
     */
    ~KVDiskFile();

//     /**
//      * rewind file
//      */
//     void rewind();
//     
//     /**
//      * flush all pending writes to disk
//      */
//     void sync();
// 
//     /**
//      * close file
//      */
//     void close();

    /**
     * file system name of this file
     */
    char *name();
    
protected:
    VFile       *m_vfile;    
};

#endif
