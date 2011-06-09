#ifndef KVTSERIALIZATION_H
#define KVTSERIALIZATION_H

#include "Global.h"

uint32_t serialize_len(const char *key, const char *value);

bool    serialize(char *buf, uint32_t buflen, const char *key, const char *value, uint32_t *len);

// NOTE: in case 'copy_keyvalue = false', strings pointed by 'key' and 'value' are valid only until next call of deserialize.
//       caller must copy them if he wants to use them after next call to deserialize
bool    deserialize(char *buf, uint32_t buflen, const char **key, const char **value, uint32_t *len, bool copy_keyvalue);

bool    deserialize(char *buf, uint32_t buflen, const char **key, const char **value, uint32_t *len);

#endif
