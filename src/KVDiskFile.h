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
     * open file for read and write (create if not existing)
     *
     * @return normally, should return true
     */
    bool open(char *filename);


    /**
     * open an existing file
     *
     * @return true if file exists and was succesfully opened. otherwise false
     */
    bool open_existing(char *filename);

    /**
     * open a new unique file in /tmp/ (see TMPFILEDIR, TMPFILEPREFIX in Global.h)
     *
     * @return normally, should return true
     */
    bool open_unique();

    /**
     * delete file from disk
     */
    void delete_from_disk();

//     // file system name of this file
//     char *name();    // NOTE: a VFile can be composed of several disk files. modify function to return array of names?!
//     // rewind file
//     void rewind();
//     // flush all pending writes to disk
//     void sync();
//     // close file
//     void close();

protected:

    void sanity_check();

    VFile       *m_vfile;
};

#endif
