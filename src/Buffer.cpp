#include "Global.h"
#include "Buffer.h"

#include <cstdlib>

/*============================================================================
 *                                 SimpleBuffer
 *============================================================================*/
Buffer::Buffer(uint32_t bufsize)
    : m_buf(NULL), m_buf_size(bufsize), m_bytes_in_buf(0), m_bytes_used(0)
{
    m_buf = (char *)malloc(m_buf_size);
}

/*============================================================================
 *                                ~SimpleBuffer
 *============================================================================*/
Buffer::~Buffer()
{
    free(m_buf);
}
