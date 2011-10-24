#ifndef RANGESCANNER_H
#define RANGESCANNER_H

#include <stdint.h>

class KeyValueStore;

class RangeScanner {

public:

    /**
     * constructor
     */
    RangeScanner(KeyValueStore *kvstore);

    /**
     * destructor
     */
    ~RangeScanner();

    /**
     * get tuple with key 'key'
     *
     * @return 1 if key exists, else 0
     */
    int point_get(const char *key);

    /**
     * get all tuples with keys between 'start_key' (inclusive) and
     * 'end_key' (exclusive), i.e. all keys K where: start_key <= K < end_key.
     * ('start_key' and 'end_key' are not copied).
     *
     * @return number of keys read
     */
    int range_get(const char *start_key, const char *end_key);

    /**
     * as above, but user can define whether 'start_key' and 'end_key' are
     * inclusive or not
     */
    int range_get(const char *start_key, const char *end_key, bool start_incl, bool end_incl);

protected:

    KeyValueStore *m_kvstore;
};

#endif
