#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>

class Buffer {

public:

    /**
     * constructor
     */
    Buffer(uint32_t bufsize);

    /**
     * destructor
     */
    ~Buffer();

    /**
     * return capacity of buffer (m_buf_size)
     */
    uint32_t capacity();

    /**
     * return number of bytes in buffer (m_bytes_in_buf)
     */
    uint32_t size();

    /**
     * return size of free space in buffer (m_buf_size - m_bytes_in_buf)
     */
    uint32_t free_space();

    /**
     * return number of bytes used from buffer (m_bytes_used)
     */
    uint32_t used();

    /**
     * return number of unused bytes in buffer (m_bytes_in_buf - m_bytes_used)
     */
    uint32_t unused();

    /**
     * clear contents of buffer
     */
    void clear();

    /**
     * shift all unused bytes to the beginning of buffer
     */
    void keep_unused();

    static uint32_t serialize_len(uint32_t keylen, uint32_t valuelen, uint64_t timestamp);
    bool serialize(const char *key, uint32_t keylen, const char *value, uint32_t valuelen, uint64_t timestamp, uint32_t *len);
    // NOTE: in case 'copy_keyvalue = false', strings pointed by 'key' and 'value' are valid only until next call of deserialize.
    bool deserialize(const char **key, const char **value, uint64_t *timestamp, uint32_t *len, bool copy_keyvalue);

    char     *m_buf;
    uint32_t  m_buf_size;
    uint32_t  m_bytes_in_buf;
    uint32_t  m_bytes_used;

protected:

    bool str_is_alnum(const char *str, int len);
};

#endif
