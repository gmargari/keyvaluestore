#ifndef KVTDISKFILEINPUTSTREAM_H
#define KVTDISKFILEINPUTSTREAM_H

#include "KVTInputStream.h"

class KVTDiskFile;

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
    void set_key_range(const char *start_key, const char *end_key, bool start_incl, bool end_incl);

    void set_key_range(const char *start_key, const char *end_key);

    bool read(const char **key, const char **value, uint64_t *timestamp);

    void reset();

    /**
     * set buffer size to minimum (this is the usual case, used for searching
     * <key, values> tuples on disk. typically, this sets buffer size to
     * MAX_INDEX_DIST).
     */
    void set_buf_size_min();

    /**
     * set buffer size to maximum (used for merging disk files. this sets
     * buffer size to buffer capacity -typically, SCANNERBUFSIZE).
     */
    void set_buf_size_max();

    /**
     * set buffer size to this size
     */
    void set_buf_size(uint32_t size);

    /**
     * get current buffer size
     */
    uint32_t get_buf_size();

    /**
     * get buffer capacity
     */
    uint32_t get_buf_capacity();

protected:

    void sanity_check();

    KVTDiskFile *m_kvtdiskfile;
    char        *m_buf;
    uint32_t     m_buf_capacity;
    uint32_t     m_buf_size;
    uint32_t     m_bytes_in_buf;
    uint32_t     m_bytes_used;

    const char  *m_start_key;
    const char  *m_end_key;
    bool         m_start_incl;
    bool         m_end_incl;
};

#endif
