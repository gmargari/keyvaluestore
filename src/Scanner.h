// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_SCANNER_H_
#define SRC_SCANNER_H_

#include <stdint.h>

#include "./Global.h"

class KeyValueStore;

class Scanner {
  public:
    /**
     * constructor
     */
    Scanner(KeyValueStore *kvstore);

    /**
     * destructor
     */
    ~Scanner();

    /**
     * get tuple with key 'key'
     *
     * @return 1 if key exists, else 0
     */
    int point_get(const char *key, uint32_t keylen);

    /**
     * get at most 'max_entries' KVs with keys between 'start_key'
     * and 'end_key'. 'start_key' and 'end_key' can be inclusive or not,
     * based on 'start_incl' and 'end_incl'.
     * - max_entries = 0: disable 'max_entries' options
     * - start_key = NULL: disable 'start_key' (e.g. 'start_key' = -oo)
     * - end_key = NULL: disable 'end_key' (e.g. 'end_key' = +oo)
     * ('start_key' and 'end_key' are not copied.)
     *
     * @return number of keys read
     */
    int range_get(const char *start_key, uint32_t start_keylen,
                  const char *end_key, uint32_t end_keylen,
                  bool start_incl, bool end_incl, int max_entries);

    /**
     * as above, but return at most 'max_entries' KVs with keys in range
     * [start_key, end_key)
     */
    int range_get(const char *start_key, uint32_t start_keylen,
                  const char *end_key, uint32_t end_keylen,
                  int max_entries);

    /**
     * as above, but return all KVs with keys in range [start_key, end_key)
     */
    int range_get(const char *start_key, uint32_t start_keylen,
                  const char *end_key, uint32_t end_keylen);

    // Undefined methods (just remove Weffc++ warning)
    Scanner(const Scanner&);
    Scanner& operator=(const Scanner&);

  private:
    KeyValueStore *m_kvstore;
};

#endif  // SRC_SCANNER_H_
