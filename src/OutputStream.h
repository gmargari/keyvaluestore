#ifndef KVTOUTPUTSTREAM_H
#define KVTOUTPUTSTREAM_H

#include <stdint.h>
#include <unistd.h>

class OutputStream {

public:

    virtual ~OutputStream() { };

    /**
     * reset stream
     */
    virtual void reset() = 0;

    /**
     * append <key, value, timestamp> tuple to stream
     *
     * @return false if could not append tuple to stream. else, true.
     */
    virtual bool append(const char *key, size_t keylen, const char *value, size_t valuelen, uint64_t timestamp) = 0;

    /**
     * close stream (e.g. flush to disk pending writes etc)
     */
    virtual void close() = 0;

protected:

};

#endif
