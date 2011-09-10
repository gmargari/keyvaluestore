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
     * write <key, value, timestamp> tuple to stream
     *
     * @return false if could not write tuple to stream. else, true.
     */
    virtual bool write(const char *key, size_t keylen, const char *value, size_t valuelen, uint64_t timestamp) = 0;

    /**
     * flush output stream buffer
     */
    virtual void flush() = 0;

protected:

};

#endif
