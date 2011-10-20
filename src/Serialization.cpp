#include "Global.h"
#include "Serialization.h"

#include "Buffer.h"

#include <cassert>
#include <cstring>
#include <cctype>
#include <cstdlib>

/*============================================================================
 *                               str_is_alnum
 *============================================================================*/
bool str_is_alnum(const char *str, int len)
{
    for (int i = 0; i < len; i++)
        if (!isalnum(str[i]))
            return false;

    return true;
}

/*============================================================================
 *                               serialize_len
 *============================================================================*/
uint32_t serialize_len(uint32_t keylen, uint32_t valuelen, uint64_t timestamp)
{
    return (sizeof(keylen) + sizeof(valuelen) + sizeof(timestamp) + keylen + valuelen + 2);
}

/*============================================================================
 *                                serialize
 *============================================================================*/
bool serialize(Buffer *buf, const char *key, uint32_t keylen,
               const char *value, uint32_t valuelen, uint64_t timestamp, uint32_t *len)
{
    uint32_t used = 0;
    char *ptr;
    uint32_t buflen;

    assert(key && value);
    assert(keylen <= MAX_KVTSIZE);
    assert(valuelen <= MAX_KVTSIZE);
    if (DBGLVL > 1) {
        assert(str_is_alnum(key, keylen));
        assert(str_is_alnum(value, valuelen));
    }

    ptr = buf->m_buf + buf->m_bytes_in_buf;
    buflen = buf->m_buf_size - buf->m_bytes_in_buf;

    if ((*len = serialize_len(keylen, valuelen, timestamp)) > buflen) {
        return false;
    }

    ENCODE_NUM(ptr, keylen, used);
    ENCODE_NUM(ptr, valuelen, used);
    ENCODE_NUM(ptr, timestamp, used);
    ENCODE_STR(ptr, key, keylen + 1, used);
    ENCODE_STR(ptr, value, valuelen + 1, used);

    assert(used == *len);
    assert(ptr[sizeof(keylen) + sizeof(valuelen) + sizeof(timestamp) + keylen] == '\0');
    assert(ptr[sizeof(keylen) + sizeof(valuelen) + sizeof(timestamp) + keylen + 1 + valuelen] == '\0');

    buf->m_bytes_in_buf += *len;

#if DBGLVL > 1
    {
        const char *kkk, *vvv;
        uint64_t ts;
        uint32_t lll, tmpused = buf->m_bytes_in_buf;
        buf->m_bytes_used = buf->m_bytes_in_buf - *len;
        assert(deserialize(buf, &kkk, &vvv, &ts, &lll, false));
        buf->m_bytes_used = tmpused;
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
bool deserialize(Buffer *buf, const char **key, const char **value, uint64_t *timestamp, uint32_t *len, bool copy_keyvalue)
{
    uint32_t used = 0, keylen, valuelen;
    char *tmpkey, *tmpvalue;
    const char *ptr;
    uint32_t buflen;

    ptr = buf->m_buf + buf->m_bytes_used;
    buflen = buf->m_bytes_in_buf - buf->m_bytes_used;

    if (buflen < sizeof(keylen) + sizeof(valuelen) + sizeof(*timestamp)) {
        return false;
    }

    DECODE_NUM(ptr, keylen, used);
    DECODE_NUM(ptr, valuelen, used);
    DECODE_NUM(ptr, *timestamp, used);

    if (buflen < serialize_len(keylen, valuelen, *timestamp)) {
        return false;
    }

    if (copy_keyvalue) {
        tmpkey = (char *)malloc(keylen + 1);
        tmpvalue = (char *)malloc(valuelen + 1);
        assert(tmpkey && tmpvalue);
        DECODE_STR(ptr, tmpkey, keylen + 1, used);
        DECODE_STR(ptr, tmpvalue, valuelen + 1, used);
    } else {
        assert(sizeof(keylen) + sizeof(valuelen) + sizeof(*timestamp) == used);
        tmpkey = (char *)ptr + sizeof(keylen) + sizeof(valuelen) + sizeof(*timestamp);
        tmpvalue = tmpkey + keylen + 1;
    }

    *len = serialize_len(keylen, valuelen, *timestamp);
    *key = tmpkey;
    *value = tmpvalue;
    buf->m_bytes_used += *len;

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
 *                                deserialize
 *============================================================================*/
bool deserialize(Buffer *buf, const char **key, uint64_t *timestamp, const char **value, uint32_t *len)
{
    return deserialize(buf, key, value, timestamp, len, true);
}
