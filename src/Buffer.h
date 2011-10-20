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
