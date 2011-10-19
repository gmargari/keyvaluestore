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

    char     *m_buf;
    uint32_t  m_buf_size;
    uint32_t  m_bytes_in_buf;
    uint32_t  m_bytes_used;
};

#endif
