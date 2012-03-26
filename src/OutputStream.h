// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_OUTPUTSTREAM_H_
#define SRC_OUTPUTSTREAM_H_

#include <stdint.h>

#include "./leveldb/slice.h"

class OutputStream {
  public:
    /**
     * constructor
     */
    OutputStream() { }

    /**
     * destructor (define as 'virtual' so it'll call derived class destructor)
     */
    virtual ~OutputStream() { }

    /**
     * append <key, value, timestamp> tuple to stream
     *
     * @return false if could not append tuple to stream. else, true.
     */
    virtual bool append(Slice key, Slice value, uint64_t timestamp) = 0;

    /**
     * close stream (e.g. flush to disk pending writes etc)
     */
    virtual void close() = 0;
};

#endif  // SRC_OUTPUTSTREAM_H_
