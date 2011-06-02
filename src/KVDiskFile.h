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

    /**
     * delete file from disk 
     */
    void delete_from_disk();
    
//     /**
//      * file system name of this file
//      */
//     char *name();    // NOTE: a VFile can be composed of several disk files. modify function to return array of names?!
// 
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
    
protected:

    void sanity_check();

    VFile       *m_vfile;
};

#endif
