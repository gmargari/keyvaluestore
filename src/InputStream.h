// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_INPUTSTREAM_H_
#define SRC_INPUTSTREAM_H_

#include <stdint.h>

#include "./Slice.h"

class InputStream {
  public:
    /**
     * constructor
     */
    InputStream() { }

    /**
     * destructor (define as 'virtual' so it'll call derived class destructor)
     */
    virtual ~InputStream() { }

    /**
     * return only KVs with keys between 'start_key' (inclusive) and 'end_key'
     * (exclusive), i.e. all keys K where: start_key <= K < end_key.
     * 'start_key' and 'end_key' are not copied.
     */
    virtual void set_key_range(Slice start_key, Slice end_key) = 0;

    /**
     * as above, but user can define whether 'start_key' and 'end_key' are
     * inclusive or not
     */
    virtual void set_key_range(Slice start_key, Slice end_key, bool start_incl,
                               bool end_incl) = 0;

    /**
     * get next <key, value, timestamp> tuple from stream
     *
     * @return false if no tuple left. else, true.
     */
    virtual bool read(Slice *key, Slice *value, uint64_t *timestamp) = 0;
};

#endif  // SRC_INPUTSTREAM_H_
