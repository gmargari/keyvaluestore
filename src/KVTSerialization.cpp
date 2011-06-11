#include "Global.h"
#include "KVTSerialization.h"

#include <cassert>
#include <cstring>
#include <cstdlib>

// TODO: make these portable!
#define ENCODE_NUM(_buf_, _num_, _used_) do { memcpy((_buf_) + _used_,  &(_num_), sizeof(_num_)); _used_ += sizeof(_num_); } while (0)
#define DECODE_NUM(_buf_, _num_, _used_) do { memcpy(&(_num_), (_buf_) + _used_, sizeof(_num_)); _used_ += sizeof(_num_); } while (0)
#define ENCODE_STR(_buf_, _str_, _len_, _used_) do { memcpy((_buf_) + _used_,  (_str_), (_len_)); _used_ += (_len_); } while (0)
#define DECODE_STR(_buf_, _str_, _len_, _used_) do { memcpy((_str_), (_buf_) + _used_, (_len_)); _used_ += (_len_); } while (0)

/*=======================================================================*
 *                              serialize_len
 *=======================================================================*/
uint32_t serialize_len(const char *key, const char *value, uint64_t timestamp) {
    return (2 * sizeof(uint32_t) + sizeof(timestamp) + strlen(key) + 1 + strlen(value) + 1);
}

/*=======================================================================*
 *                                serialize
 *=======================================================================*/
bool serialize(char *buf, uint32_t buflen, const char *key, const char *value, uint64_t timestamp, uint32_t *len) {
    uint32_t used = 0,
             keylen = strlen(key),
             valuelen = strlen(value);

    if ((*len = serialize_len(key, value, timestamp)) > buflen) {
        return false;
    }
    assert(key && value);
    ENCODE_NUM(buf, keylen, used);
    ENCODE_NUM(buf, valuelen, used);
    ENCODE_NUM(buf, timestamp, used);
    ENCODE_STR(buf, key, keylen + 1, used);
    ENCODE_STR(buf, value, valuelen + 1, used);
    assert(used == *len);
    assert(buf[sizeof(keylen) + sizeof(valuelen) + sizeof(timestamp) + keylen] == '\0');
    assert(buf[sizeof(keylen) + sizeof(valuelen) + sizeof(timestamp) + keylen + 1 + valuelen] == '\0');

    return true;
}

/*=======================================================================*
 *                               deserialize
 *=======================================================================*/
bool deserialize(char *buf, uint32_t buflen, const char **key, const char **value, uint64_t *timestamp, uint32_t *len, bool copy_keyvalue)
{
    uint32_t used = 0,
             keylen,
             valuelen;
    char *tmpkey, *tmpvalue;

    if (buflen < sizeof(keylen) + sizeof(valuelen) + sizeof(*timestamp)) {
        return false;
    }

    DECODE_NUM(buf, keylen, used);
    DECODE_NUM(buf, valuelen, used);
    DECODE_NUM(buf, *timestamp, used);

    if (buflen < sizeof(keylen) + sizeof(valuelen) + keylen + valuelen + 2) {
        return false;
    }

    if (copy_keyvalue) {
        // can have extra fields 'keycapacity' and 'valuecapacity',
        // and realloc key and value only if their capacity needs to
        // be extended
        tmpkey = (char *)malloc(keylen + 1);
        tmpvalue = (char *)malloc(valuelen + 1);
        assert(tmpkey && tmpvalue);
        DECODE_STR(buf, tmpkey, keylen + 1, used);
        DECODE_STR(buf, tmpvalue, valuelen + 1, used);
    } else {
        assert(sizeof(keylen) + sizeof(valuelen) + sizeof(*timestamp) == used);
        tmpkey = buf + sizeof(keylen) + sizeof(valuelen) + sizeof(*timestamp);
        tmpvalue = tmpkey + keylen + 1;
    }

    *len = sizeof(keylen) + sizeof(valuelen) + sizeof(*timestamp) + keylen + valuelen + 2;
    *key = tmpkey;
    *value = tmpvalue;

    assert(*key);
    assert(*value);
    assert((*key)[keylen] == '\0');
    assert((*value)[valuelen] == '\0');

    return true;
}

/*=======================================================================*
 *                               deserialize
 *=======================================================================*/
bool deserialize(char *buf, uint32_t buflen, const char **key, uint64_t *timestamp, const char **value, uint32_t *len) {
    return deserialize(buf, buflen, key, value, timestamp, len, true);
}