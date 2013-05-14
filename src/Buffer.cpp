// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#include "./Global.h"
#include "./Buffer.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "./VFile.h"

// TODO: make these portable
#define ENCODE_NUM(buf_, num_, used_)                   \
    STMT(memcpy((buf_) + used_, &(num_), sizeof(num_)); \
         used_ += sizeof(num_);                         \
    )

#define DECODE_NUM(buf_, num_, used_)                   \
    STMT(memcpy(&(num_), (buf_) + used_, sizeof(num_)); \
         used_ += sizeof(num_);                         \
    )

#define ENCODE_STR(buf_, str_, len_, used_)             \
    STMT(memcpy((buf_) + used_, (str_), (len_));        \
         used_ += (len_);                               \
    )

#define DECODE_STR(buf_, str_, len_, used_)             \
    STMT(memcpy((str_), (buf_) + used_, (len_));        \
         used_ += (len_);                               \
    )

/*============================================================================
 *                                 Buffer
 *============================================================================*/
Buffer::Buffer(uint32_t bufsize)
    : m_buf(NULL), m_buf_size(bufsize), m_bytes_in_buf(0), m_bytes_used(0),
      m_buf_allocated(true) {
    m_buf = (char *)malloc(m_buf_size);
}

/*============================================================================
 *                                 Buffer
 *============================================================================*/
Buffer::Buffer(char *buf, uint32_t bufsize)
    : m_buf(NULL), m_buf_size(bufsize), m_bytes_in_buf(0), m_bytes_used(0),
      m_buf_allocated(false) {
    m_buf = buf;
}

/*============================================================================
 *                                ~Buffer
 *============================================================================*/
Buffer::~Buffer() {
    if (m_buf_allocated) {
        free(m_buf);
    }
}

/*============================================================================
 *                                  fill
 *============================================================================*/
uint32_t Buffer::fill(VFile *vfile, uint32_t bytes, off_t offs) {
    uint32_t bytes_read = vfile->fs_pread(m_buf + size(), bytes, offs);
    m_bytes_in_buf += bytes_read;
    return bytes_read;
}

/*============================================================================
 *                                  fill
 *============================================================================*/
uint32_t Buffer::fill(VFile *vfile, off_t offs) {
    return fill(vfile, free_space(), offs);
}

/*============================================================================
 *                                 append
 *============================================================================*/
uint32_t Buffer::append(VFile *vfile) {
    uint32_t bytes_written = vfile->fs_append(m_buf, size());
    assert(bytes_written == size());
    clear();
    return bytes_written;
}

/*============================================================================
 *                                keep_unused
 *============================================================================*/
void Buffer::keep_unused() {
    memmove(m_buf, m_buf + m_bytes_used, m_bytes_in_buf - m_bytes_used);
    m_bytes_in_buf -= m_bytes_used;
    m_bytes_used = 0;
}

/*============================================================================
 *                                serialize
 *============================================================================*/
bool Buffer::serialize(Slice key, Slice value, uint64_t timestamp,
                       uint32_t *len) {
    uint32_t bytes_used = 0;
    char *ptr;
    uint32_t buflen;
    uint32_t keylen = key.size(), valuelen = value.size();

    assert(key.data() && value.data());
    assert(keylen <= MAX_KEY_SIZE);
    assert(valuelen <= MAX_VALUE_SIZE);
    if (DBGLVL > 1) {
        assert(slice_is_alnum(key));
        assert(slice_is_alnum(value));
    }

    ptr = m_buf + m_bytes_in_buf;
    buflen = m_buf_size - m_bytes_in_buf;

    if ((*len = serialize_len(keylen, valuelen, timestamp)) > buflen) {
        return false;
    }

    ENCODE_NUM(ptr, keylen, bytes_used);
    ENCODE_NUM(ptr, valuelen, bytes_used);
    ENCODE_NUM(ptr, timestamp, bytes_used);
    ENCODE_STR(ptr, key.data(), keylen + 1, bytes_used);
    ENCODE_STR(ptr, value.data(), valuelen + 1, bytes_used);

    m_bytes_in_buf += *len;

    assert(bytes_used == *len);
    assert(ptr[sizeof(keylen) + sizeof(valuelen) + sizeof(timestamp) + keylen]
             == '\0');
    assert(ptr[sizeof(keylen) + sizeof(valuelen) + sizeof(timestamp) + keylen
             + 1 + valuelen] == '\0');
#if DBGLVL > 1
    Slice kkk, vvv;
    uint64_t ts;
    uint32_t tmpused = m_bytes_in_buf;
    m_bytes_used = m_bytes_in_buf - *len;
    assert(deserialize(&kkk, &vvv, &ts, false));
    m_bytes_used = tmpused;
    assert(strcmp(kkk.data(), key.data()) == 0);
    assert(strcmp(vvv.data(), value.data()) == 0);
    assert(ts == timestamp);
#endif

    return true;
}

/*============================================================================
 *                                deserialize
 *============================================================================*/
bool Buffer::deserialize(Slice *key, Slice *value, uint64_t *timestamp,
                         bool copy_keyvalue) {
    uint32_t bytes_used = 0;
    char *tmpkey, *tmpvalue;
    const char *ptr;
    uint32_t buflen;
    uint32_t len;
    uint32_t keylen, valuelen;

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
        assert(sizeof(keylen) + sizeof(valuelen) + sizeof(*timestamp)
                 == bytes_used);
        tmpkey = (char *)ptr + sizeof(keylen) + sizeof(valuelen)
                    + sizeof(*timestamp);
        tmpvalue = tmpkey + keylen + 1;
    }

    len = serialize_len(keylen, valuelen, *timestamp);
    m_bytes_used += len;

    *key = Slice(tmpkey, keylen);
    *value = Slice(tmpvalue, valuelen);

    assert(key->data());
    assert(value->data());
    assert(key->size() <= MAX_KEY_SIZE);
    assert(value->size() <= MAX_VALUE_SIZE);
    assert(key->data() + key->size() < ptr + buflen);
    assert(key->data()[key->size()] == '\0');
    assert(value->data() + value->size() < ptr + buflen);
    assert(value->data()[value->size()] == '\0');
    assert(strlen(key->data()) == key->size());
    assert(strlen(value->data()) == value->size());
    if (DBGLVL > 1) {
        assert(slice_is_alnum(*key));
        assert(slice_is_alnum(*value));
    }

    return true;
}

/*============================================================================
 *                               str_is_alnum
 *============================================================================*/
bool Buffer::slice_is_alnum(Slice key) {
    for (int i = 0; i < (int)key.size(); i++)
        if (!isalnum(key.data()[i]))
            return false;

    return true;
}
