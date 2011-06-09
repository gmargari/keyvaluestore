#ifndef KVTDISKFILEINPUTSTREAM_H
#define KVTDISKFILEINPUTSTREAM_H

#include "Global.h"
#include "KVTInputStream.h"
#include "KVTDiskFile.h"

class KVTDiskFileInputStream: public KVTInputStream {

public:

    /**
     * constructor
     */
    KVTDiskFileInputStream(KVTDiskFile *file);

    /**
     * destructor
     */
    ~KVTDiskFileInputStream();

    // inherited from KVTInputStream
    /**
     * reset scanner
     */
    void reset();

    /**
     * get next <key, value> pair from stream
     * (pointers are valid only until next call to function. if caller wants to
     * use them after next call, he must copy key and value)
     *
     * @return false if no <k,v> pair left. else, true.
     */
    bool read(const char **key, const char **value);

protected:

    void sanity_check();

    KVTDiskFile *m_kvtdiskfile;
    char       	*m_buf;
    uint32_t     m_buf_size;
    uint32_t     m_bytes_in_buf;
    uint32_t     m_bytes_used;
};

#endif
