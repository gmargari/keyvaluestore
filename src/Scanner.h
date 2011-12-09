#ifndef SCANNER_H
#define SCANNER_H

#include <stdint.h>

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
     * get all tuples with keys between 'start_key' (inclusive) and
     * 'end_key' (exclusive), i.e. all keys K where: start_key <= K < end_key.
     * ('start_key' and 'end_key' are not copied).
     *
     * @return number of keys read
     */
    int range_get(const char *start_key, uint32_t start_keylen, const char *end_key, uint32_t end_keylen);

    /**
     * as above, but user can define whether 'start_key' and 'end_key' are
     * inclusive or not
     */
    int range_get(const char *start_key, uint32_t start_keylen, const char *end_key, uint32_t end_keylen, bool start_incl, bool end_incl);

    // Undefined methods (just remove Weffc++ warning)
    Scanner(const Scanner&);
    Scanner& operator=(const Scanner&);

protected:

    KeyValueStore *m_kvstore;
};

#endif
