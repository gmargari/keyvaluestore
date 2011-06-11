#ifndef KVTDISKFILE_H
#define KVTDISKFILE_H

#include "Global.h"
#include "VFile.h"
#include "VFileIndex.h"
#include "KVTSerialization.h"

class KVTDiskFile {

friend class KVTDiskFileInputStream;
friend class KVTDiskFileOutputStream;

public:

    /**
     * constructor
     */
    KVTDiskFile();

    /**
     * destructor
     */
    ~KVTDiskFile();

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

    /**
     * number of keys (or tuples) stored on disk file
     */
    uint64_t num_keys();

    /**
     * size of disk file
     */
    uint64_t size();

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
    VFileIndex   m_vfile_index;
    uint64_t     m_vfile_numkeys;
};

#endif
