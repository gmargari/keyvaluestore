// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_OUTPUTSTREAM_H_
#define SRC_OUTPUTSTREAM_H_

#include <stdint.h>

class OutputStream {
  public:
    /**
     * constructor
     */
    OutputStream() { }

    /**
     * destructor (define as 'virtual' so it will call destructors of derived classes)
     */
    virtual ~OutputStream() { }

    /**
     * append <key, value, timestamp> tuple to stream
     *
     * @return false if could not append tuple to stream. else, true.
     */
    virtual bool append(const char *key, uint32_t keylen, const char *value, uint32_t valuelen, uint64_t timestamp) = 0;

    /**
     * close stream (e.g. flush to disk pending writes etc)
     */
    virtual void close() = 0;
};

#endif  // SRC_OUTPUTSTREAM_H_
