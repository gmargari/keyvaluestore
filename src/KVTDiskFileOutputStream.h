#ifndef KVTDISKFILEOUTPUTSTREAM_H
#define KVTDISKFILEOUTPUTSTREAM_H

#include "KVTOutputStream.h"

#include <cstdio>

class KVTDiskFile;
class VFile;
class VFileIndex;

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
     * flush output stream buffer. user must call this function after
     * writing all data in order to ensure data durability and to properly
     * create index of underlying vfile.
     */
    void flush();

protected:

    void sanity_check();

    VFile       *m_vfile;         // pointer to vfile
    VFileIndex  *m_vfile_index;   // pointer to vfile index
    uint64_t    *m_vfile_numkeys; // pointer to vfile's 'm_vfile_numkeys'
    uint64_t     m_vfile_size;    // size of vfile      // TODO: remove and use vfile->fs_size(), when fs_size() is implemented without using syscalls

    char       	*m_buf;           // buffer used to serialize entries
    uint32_t     m_buf_size;      // buffer size
    uint32_t     m_bytes_in_buf;  // total bytes in byffer
    uint32_t     m_bytes_used;    // bytes already used from buffer

    char        *m_last_key;      // last key written to disk
    off_t        m_last_offs;     // offset of last key written to disk
    off_t        m_last_idx_offs; // last offset inserted in index
};

#endif
