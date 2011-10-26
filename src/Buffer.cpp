#include "Global.h"
#include "Buffer.h"

#include "VFile.h"

#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cctype>

// TODO: make these portable!
#define ENCODE_NUM(_buf_, _num_, _used_) do { memcpy((_buf_) + _used_,  &(_num_), sizeof(_num_)); _used_ += sizeof(_num_); } while (0)
#define DECODE_NUM(_buf_, _num_, _used_) do { memcpy(&(_num_), (_buf_) + _used_, sizeof(_num_)); _used_ += sizeof(_num_); } while (0)
#define ENCODE_STR(_buf_, _str_, _len_, _used_) do { memcpy((_buf_) + _used_,  (_str_), (_len_)); _used_ += (_len_); } while (0)
#define DECODE_STR(_buf_, _str_, _len_, _used_) do { memcpy((_str_), (_buf_) + _used_, (_len_)); _used_ += (_len_); } while (0)

/*============================================================================
 *                                 SimpleBuffer
 *============================================================================*/
Buffer::Buffer(uint32_t bufsize)
    : m_buf(NULL), m_buf_size(bufsize), m_bytes_in_buf(0), m_bytes_used(0),
      m_buf_allocated(true)
{
    m_buf = (char *)malloc(m_buf_size);
}

/*============================================================================
 *                                 SimpleBuffer
 *============================================================================*/
Buffer::Buffer(char *buf, uint32_t bufsize)
    : m_buf(NULL), m_buf_size(bufsize), m_bytes_in_buf(0), m_bytes_used(0),
      m_buf_allocated(false)
{
    m_buf = buf;
}

/*============================================================================
 *                                ~SimpleBuffer
 *============================================================================*/
Buffer::~Buffer()
{
    if (m_buf_allocated) {
        free(m_buf);
    }
}

/*============================================================================
 *                                capacity
 *============================================================================*/
uint32_t Buffer::capacity()
{
    return m_buf_size;
}

/*============================================================================
 *                                  size
 *============================================================================*/
uint32_t Buffer::size()
{
    return m_bytes_in_buf;
}

/*============================================================================
 *                               free_space
 *============================================================================*/
uint32_t Buffer::free_space()
{
    return m_buf_size - m_bytes_in_buf;
}

/*============================================================================
 *                                  used
 *============================================================================*/
uint32_t Buffer::used()
{
    return m_bytes_used;
}

/*============================================================================
 *                                 unused
 *============================================================================*/
uint32_t Buffer::unused()
{
    return m_bytes_in_buf - m_bytes_used;
}

/*============================================================================
 *                                  clear
 *============================================================================*/
void Buffer::clear()
{
    m_bytes_in_buf = 0;
    m_bytes_used = 0;
}

/*============================================================================
 *                                  fill
 *============================================================================*/
uint32_t Buffer::fill(VFile *vfile, uint32_t bytes, off_t offs)
{
    uint32_t bytes_read;

    bytes_read = vfile->fs_pread(m_buf + size(), bytes, offs);
    m_bytes_in_buf += bytes_read;

    return bytes_read;
}

/*============================================================================
 *                                  fill
 *============================================================================*/
uint32_t Buffer::fill(VFile *vfile, off_t offs)
{
    return fill(vfile, free_space(), offs);
}

/*============================================================================
 *                                  flush
 *============================================================================*/
uint32_t Buffer::flush(VFile *vfile, off_t offs)
{
    uint32_t bytes_written;

    bytes_written = vfile->fs_pwrite(m_buf, size(), offs);
    assert(bytes_written == size());
    clear();

    return bytes_written;
}

/*============================================================================
 *                                keep_unused
 *============================================================================*/
void Buffer::keep_unused()
{
    uint32_t unused_bytes = m_bytes_in_buf - m_bytes_used;

    memmove(m_buf, m_buf + m_bytes_used, unused_bytes);
    m_bytes_in_buf = unused_bytes;
    m_bytes_used = 0;
}

/*============================================================================
 *                               serialize_len
 *============================================================================*/
uint32_t Buffer::serialize_len(uint32_t keylen, uint32_t valuelen, uint64_t timestamp)
{
    return (sizeof(keylen) + sizeof(valuelen) + sizeof(timestamp) + keylen + valuelen + 2);
}

/*============================================================================
 *                                serialize
 *============================================================================*/
bool Buffer::serialize(const char *key, uint32_t keylen, const char *value,
                       uint32_t valuelen, uint64_t timestamp, uint32_t *len)
{
    uint32_t bytes_used = 0;
    char *ptr;
    uint32_t buflen;

    assert(key && value);
    assert(keylen <= MAX_KVTSIZE);
    assert(valuelen <= MAX_KVTSIZE);
    if (DBGLVL > 1) {
        assert(str_is_alnum(key, keylen));
        assert(str_is_alnum(value, valuelen));
    }

    ptr = m_buf + m_bytes_in_buf;
    buflen = m_buf_size - m_bytes_in_buf;

    if ((*len = serialize_len(keylen, valuelen, timestamp)) > buflen) {
        return false;
    }

    ENCODE_NUM(ptr, keylen, bytes_used);
    ENCODE_NUM(ptr, valuelen, bytes_used);
    ENCODE_NUM(ptr, timestamp, bytes_used);
    ENCODE_STR(ptr, key, keylen + 1, bytes_used);
    ENCODE_STR(ptr, value, valuelen + 1, bytes_used);

    assert(bytes_used == *len);
    assert(ptr[sizeof(keylen) + sizeof(valuelen) + sizeof(timestamp) + keylen] == '\0');
    assert(ptr[sizeof(keylen) + sizeof(valuelen) + sizeof(timestamp) + keylen + 1 + valuelen] == '\0');

    m_bytes_in_buf += *len;

#if DBGLVL > 1
    {
        const char *kkk, *vvv;
        uint64_t ts;
        uint32_t lll, tmpused = m_bytes_in_buf;
        m_bytes_used = m_bytes_in_buf - *len;
        assert(deserialize(&kkk, &vvv, &ts, &lll, false));
        m_bytes_used = tmpused;
        assert(strcmp(kkk, key)==0);
        assert(strcmp(vvv, value)==0);
        assert(ts == timestamp);
        assert(lll == *len);
    }
#endif

    return true;
}

/*============================================================================
 *                                deserialize
 *============================================================================*/
bool Buffer::deserialize(const char **key, const char **value, uint64_t *timestamp, uint32_t *len, bool copy_keyvalue)
{
    uint32_t bytes_used = 0, keylen, valuelen;
    char *tmpkey, *tmpvalue;
    const char *ptr;
    uint32_t buflen;

    ptr = m_buf + m_bytes_used;
    buflen = m_bytes_in_buf - m_bytes_used;

    if (buflen < sizeof(keylen) + sizeof(valuelen) + sizeof(*timestamp)) {
        return false;
    }

    DECODE_NUM(ptr, keylen, bytes_used);
    DECODE_NUM(ptr, valuelen, bytes_used);
    DECODE_NUM(ptr, *timestamp, bytes_used);

    if (buflen < serialize_len(keylen, valuelen, *timestamp)) {
        return false;
    }

    if (copy_keyvalue) {
        tmpkey = (char *)malloc(keylen + 1);
        tmpvalue = (char *)malloc(valuelen + 1);
        assert(tmpkey && tmpvalue);
        DECODE_STR(ptr, tmpkey, keylen + 1, bytes_used);
        DECODE_STR(ptr, tmpvalue, valuelen + 1, bytes_used);
    } else {
        assert(sizeof(keylen) + sizeof(valuelen) + sizeof(*timestamp) == bytes_used);
        tmpkey = (char *)ptr + sizeof(keylen) + sizeof(valuelen) + sizeof(*timestamp);
        tmpvalue = tmpkey + keylen + 1;
    }

    *len = serialize_len(keylen, valuelen, *timestamp);
    *key = tmpkey;
    *value = tmpvalue;
    m_bytes_used += *len;

    assert(*key);
    assert(*value);
    assert(keylen <= MAX_KVTSIZE);
    assert(valuelen <= MAX_KVTSIZE);
    assert(*key + keylen < ptr + buflen);
    assert((*key)[keylen] == '\0');
    assert(*value + valuelen < ptr + buflen);
    assert((*value)[valuelen] == '\0');
    assert(strlen(*key) == keylen);
    assert(strlen(*value) == valuelen);
    if (DBGLVL > 1) {
        assert(str_is_alnum(*key, strlen(*key)));
        assert(str_is_alnum(*value, strlen(*value)));
    }

    return true;
}

/*============================================================================
 *                             undo_deserialize
 *============================================================================*/
void Buffer::undo_deserialize(const char *key, const char *value, uint64_t timestamp)
{
    m_bytes_used -= serialize_len(strlen(key), strlen(value), timestamp);
}

/*============================================================================
 *                               str_is_alnum
 *============================================================================*/
bool Buffer::str_is_alnum(const char *str, int len)
{
    for (int i = 0; i < len; i++)
        if (!isalnum(str[i]))
            return false;

    return true;
}
