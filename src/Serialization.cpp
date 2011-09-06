#include "Global.h"
#include "Serialization.h"

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
bool serialize(char *buf, uint32_t buflen, const char *key, uint32_t keylen,
               const char *value, uint32_t valuelen, uint64_t timestamp, uint32_t *len)
{
    uint32_t used = 0;

    assert(key && value);
    assert(keylen <= MAX_KVTSIZE);
    assert(valuelen <= MAX_KVTSIZE);
    if (DBGLVL > 1) {
        assert(str_is_alnum(key, keylen));
        assert(str_is_alnum(value, valuelen));
    }

    if ((*len = serialize_len(keylen, valuelen, timestamp)) > buflen) {
        return false;
    }

    ENCODE_NUM(buf, keylen, used);
    ENCODE_NUM(buf, valuelen, used);
    ENCODE_NUM(buf, timestamp, used);
    ENCODE_STR(buf, key, keylen + 1, used);
    ENCODE_STR(buf, value, valuelen + 1, used);

    assert(used == *len);
    assert(buf[sizeof(keylen) + sizeof(valuelen) + sizeof(timestamp) + keylen] == '\0');
    assert(buf[sizeof(keylen) + sizeof(valuelen) + sizeof(timestamp) + keylen + 1 + valuelen] == '\0');
#if DBGLVL > 1
    {
        const char *kkk, *vvv;
        uint64_t ts;
        uint32_t lll;
        assert(deserialize(buf, buflen, &kkk, &vvv, &ts, &lll, false));
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
bool deserialize(const char *buf, uint32_t buflen, const char **key, const char **value, uint64_t *timestamp, uint32_t *len, bool copy_keyvalue)
{
    uint32_t used = 0, keylen, valuelen;
    char *tmpkey, *tmpvalue;

    if (buflen < sizeof(keylen) + sizeof(valuelen) + sizeof(*timestamp)) {
        return false;
    }

    DECODE_NUM(buf, keylen, used);
    DECODE_NUM(buf, valuelen, used);
    DECODE_NUM(buf, *timestamp, used);

    if (buflen < serialize_len(keylen, valuelen, *timestamp)) {
        return false;
    }

    if (copy_keyvalue) {
        tmpkey = (char *)malloc(keylen + 1);
        tmpvalue = (char *)malloc(valuelen + 1);
        assert(tmpkey && tmpvalue);
        DECODE_STR(buf, tmpkey, keylen + 1, used);
        DECODE_STR(buf, tmpvalue, valuelen + 1, used);
    } else {
        assert(sizeof(keylen) + sizeof(valuelen) + sizeof(*timestamp) == used);
        tmpkey = (char *)buf + sizeof(keylen) + sizeof(valuelen) + sizeof(*timestamp);
        tmpvalue = tmpkey + keylen + 1;
    }

    *len = serialize_len(keylen, valuelen, *timestamp);
    *key = tmpkey;
    *value = tmpvalue;

    assert(*key);
    assert(*value);
    assert(keylen <= MAX_KVTSIZE);
    assert(valuelen <= MAX_KVTSIZE);
    assert(*key + keylen < buf + buflen);
    assert((*key)[keylen] == '\0');
    assert(*value + valuelen < buf + buflen);
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
bool deserialize(const char *buf, uint32_t buflen, const char **key, uint64_t *timestamp, const char **value, uint32_t *len)
{
    return deserialize(buf, buflen, key, value, timestamp, len, true);
}
