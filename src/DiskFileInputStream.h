#ifndef KVTDISKFILEINPUTSTREAM_H
#define KVTDISKFILEINPUTSTREAM_H

#include "InputStream.h"

#include <sys/types.h>

class DiskFile;
class Buffer;

class DiskFileInputStream: public InputStream {

public:

    /**
     * constructor
     */
    DiskFileInputStream(DiskFile *file, uint32_t bufsize);

    /**
     * constructor
     */
    DiskFileInputStream(DiskFile *file, char *buf, uint32_t bufsize);

    /**
     * destructor
     */
    ~DiskFileInputStream();

    // inherited from InputStream
    void set_key_range(const char *start_key, const char *end_key, bool start_incl, bool end_incl);

    void set_key_range(const char *start_key, const char *end_key);

    bool read(const char **key, const char **value, uint64_t *timestamp);

protected:

    DiskFile    *m_diskfile;
    off_t        m_offs;        // offset within diskfile we currently are
    Buffer      *m_buf;         // buffer used for I/O
    const char  *m_start_key;
    const char  *m_end_key;
    bool         m_start_incl;
    bool         m_end_incl;
};

#endif
