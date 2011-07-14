#ifndef KVTDISKFILEINPUTSTREAM_H
#define KVTDISKFILEINPUTSTREAM_H

#include "InputStream.h"

class DiskFile;

class DiskFileInputStream: public InputStream {

public:

    /**
     * constructor
     */
    DiskFileInputStream(DiskFile *file, uint32_t bufsize);

    /**
     * destructor
     */
    ~DiskFileInputStream();

    // inherited from InputStream
    void set_key_range(const char *start_key, const char *end_key, bool start_incl, bool end_incl);

    void set_key_range(const char *start_key, const char *end_key);

    bool read(const char **key, const char **value, uint64_t *timestamp);

protected:

    int sanity_check();

    DiskFile *m_diskfile;
    char        *m_buf;
    uint32_t     m_buf_size;
    uint32_t     m_bytes_in_buf;
    uint32_t     m_bytes_used;

    const char  *m_start_key;
    const char  *m_end_key;
    bool         m_start_incl;
    bool         m_end_incl;
};

#endif
