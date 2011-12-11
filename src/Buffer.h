#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>
#include <sys/types.h>

class VFile;

class Buffer {

public:

    /**
     * constructor
     */
    Buffer(uint32_t bufsize);

    /**
     * constructor
     */
    Buffer(char *buf, uint32_t bufsize);

    /**
     * destructor
     */
    ~Buffer();

    /**
     * return capacity of buffer
     */
    uint32_t capacity();

    /**
     * return number of bytes in buffer
     */
    uint32_t size();

    /**
     * return size of free space in buffer
     */
    uint32_t free_space();

    /**
     * return number of bytes used from buffer
     */
    uint32_t used();

    /**
     * return number of unused bytes in buffer
     */
    uint32_t unused();

    /**
     * clear contents of buffer
     */
    void clear();

    /**
     * fill free space in buffer by reading at most 'bytes' bytes from offset
     * 'offs' of file 'vfile'
     *
     * @return number of bytes read
     */
    uint32_t fill(VFile *vfile, uint32_t bytes, off_t offs);

    /**
     * fill free space in buffer by reading bytes from offset 'offs' of file
     * 'vfile'
     *
     * @return number of bytes read
     */
    uint32_t fill(VFile *vfile, off_t offs);

    /**
     * append buffer bytes to 'vfile', clear buffer
     */
    uint32_t append(VFile *vfile);

    /**
     * shift all unused bytes to the beginning of buffer
     */
    void keep_unused();

    /**
     * return number of bytes needed to serialize <k,v,t>
     * ('keylen' = strlen(k), 'valuelen' = strlen(v), 'timestamp' = t)
     */
    static uint32_t serialize_len(uint32_t keylen, uint32_t valuelen, uint64_t timestamp);

    /**
     * serialize <k,v,t> and copy it to buffer
     */
    bool serialize(const char *key, uint32_t keylen, const char *value, uint32_t valuelen, uint64_t timestamp, uint32_t *len);

    /**
     * deserialize a <k,v,t> from buffer (in case 'copy_keyvalue = false',
     * strings pointed by 'key' and 'value' are valid only until next call of
     * function)
     */
    bool deserialize(const char **key, uint32_t *keylen, const char **value, uint32_t *valuelen, uint64_t *timestamp, bool copy_keyvalue);

    /**
     * undo deserialization previously done (rational similar to ungetc().
     * needed from DiskFileInputStream::read() in case we "accidentally"
     * deserialized a <k,v,t>)
     */
    void undo_deserialize(const char *key, uint32_t keylen, const char *value, uint32_t valuelen, uint64_t timestamp);

    // Undefined methods (just remove Weffc++ warning)
    Buffer(const Buffer&);
    Buffer& operator=(const Buffer&);

protected:

    bool str_is_alnum(const char *str, int len);

    char     *m_buf;            // pointer to char buffer
    uint32_t  m_buf_size;       // buffer capacity
    uint32_t  m_bytes_in_buf;   // number of bytes in buffer
    uint32_t  m_bytes_used;     // number of bytes used (e.g. deserialized)
    bool      m_buf_allocated;
};

/*============================================================================
 *                                capacity
 *============================================================================*/
inline uint32_t Buffer::capacity()
{
    return m_buf_size;
}

/*============================================================================
 *                                  size
 *============================================================================*/
inline uint32_t Buffer::size()
{
    return m_bytes_in_buf;
}

/*============================================================================
 *                               free_space
 *============================================================================*/
inline uint32_t Buffer::free_space()
{
    return m_buf_size - m_bytes_in_buf;
}

/*============================================================================
 *                                  used
 *============================================================================*/
inline uint32_t Buffer::used()
{
    return m_bytes_used;
}

/*============================================================================
 *                                 unused
 *============================================================================*/
inline uint32_t Buffer::unused()
{
    return m_bytes_in_buf - m_bytes_used;
}

/*============================================================================
 *                                  clear
 *============================================================================*/
inline void Buffer::clear()
{
    m_bytes_in_buf = 0;
    m_bytes_used = 0;
}

/*============================================================================
 *                               serialize_len
 *============================================================================*/
inline uint32_t Buffer::serialize_len(uint32_t keylen, uint32_t valuelen, uint64_t timestamp)
{
    return (sizeof(keylen) + sizeof(valuelen) + sizeof(timestamp) + keylen + valuelen + 2);
}

/*============================================================================
 *                             undo_deserialize
 *============================================================================*/
inline void Buffer::undo_deserialize(const char *key, uint32_t keylen, const char *value, uint32_t valuelen, uint64_t timestamp)
{
    m_bytes_used -= serialize_len(keylen, valuelen, timestamp);
}

#endif
