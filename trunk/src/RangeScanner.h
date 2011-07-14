#ifndef RANGESCANNER_H
#define RANGESCANNER_H

#include <stdint.h>

class KeyValueStore;
class PriorityInputStream;

class RangeScanner {

public:

    /**
     * constructor. user should call set_key_range() _before_ calling read().
     */
    RangeScanner(KeyValueStore *kvstore);

    /**
     * destructor
     */
    ~RangeScanner();

    /**
     * return only pairs with keys between 'start_key' (inclusive) and 'end_key'
     * (exclusive), i.e. all keys K where: start_key <= K < end_key.
     * 'start_key' and 'end_key' are not copied.
     */
    void set_key_range(const char *start_key, const char *end_key);

    /**
     * as above, but user can define whether 'start_key' and 'end_key' are
     * inclusive or not
     */
    void set_key_range(const char *start_key, const char *end_key, bool start_incl, bool end_incl);

    /**
     * get next <key, value, timestamp> from store
     *
     * @return false in there is no remaining <k,v,t> with key in the range
     *         defined in range_set_keys(), else true.
     */
    bool get_next(char **key, char **value, uint64_t *timestamp);

protected:

    int sanity_check();

    PriorityInputStream *m_pistream;
};

#endif
