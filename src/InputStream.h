#ifndef KVTINPUTSTREAM_H
#define KVTINPUTSTREAM_H

#include <stdint.h>

class InputStream {

public:

    /**
     * constructor
     */
    InputStream();

    /**
     * destructor (define as 'virtual' so it will call destructors of derived classes)
     */
    virtual ~InputStream();

    /**
     * return only pairs with keys between 'start_key' (inclusive) and 'end_key'
     * (exclusive), i.e. all keys K where: start_key <= K < end_key.
     * 'start_key' and 'end_key' are not copied.
     */
    virtual void set_key_range(const char *start_key, const char *end_key) = 0;

    /**
     * as above, but user can define whether 'start_key' and 'end_key' are
     * inclusive or not
     */
    virtual void set_key_range(const char *start_key, const char *end_key, bool start_incl, bool end_incl) = 0;

    /**
     * get next <key, value, timestamp> tuple from stream
     *
     * @return false if no tuple left. else, true.
     */
    virtual bool read(const char **key, const char **value, uint64_t *timestamp) = 0;

protected:

};

#endif
