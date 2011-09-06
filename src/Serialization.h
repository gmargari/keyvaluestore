#ifndef KVTSERIALIZATION_H
#define KVTSERIALIZATION_H

#include <cstring>
#include <stdint.h>

// TODO: make these portable!
#define ENCODE_NUM(_buf_, _num_, _used_) do { memcpy((_buf_) + _used_,  &(_num_), sizeof(_num_)); _used_ += sizeof(_num_); } while (0)
#define DECODE_NUM(_buf_, _num_, _used_) do { memcpy(&(_num_), (_buf_) + _used_, sizeof(_num_)); _used_ += sizeof(_num_); } while (0)
#define ENCODE_STR(_buf_, _str_, _len_, _used_) do { memcpy((_buf_) + _used_,  (_str_), (_len_)); _used_ += (_len_); } while (0)
#define DECODE_STR(_buf_, _str_, _len_, _used_) do { memcpy((_str_), (_buf_) + _used_, (_len_)); _used_ += (_len_); } while (0)

uint32_t serialize_len(uint32_t keylen, uint32_t valuelen, uint64_t timestamp); // TODO: make it macro or inline

bool     serialize(char *buf, uint32_t buflen, const char *key, uint32_t keylen, const char *value, uint32_t valuelen, uint64_t timestamp, uint32_t *len);

// NOTE: in case 'copy_keyvalue = false', strings pointed by 'key' and 'value' are valid only until next call of deserialize.
//       caller must copy them if he wants to use them after next call to deserialize
bool     deserialize(const char *buf, uint32_t buflen, const char **key, const char **value, uint64_t *timestamp, uint32_t *len, bool copy_keyvalue);
bool     deserialize(const char *buf, uint32_t buflen, const char **key, const char **value, uint64_t *timestamp, uint32_t *len);

#endif
