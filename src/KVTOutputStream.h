#ifndef KVTOUTPUTSTREAM_H
#define KVTOUTPUTSTREAM_H

#include "Global.h"

class KVTOutputStream {

public:

    /**
     * reset stream
     */
    virtual void reset() = 0;

    /**
     * write <key, value, timestamp> tuple to stream
     *
     * @return false if could not write tuple to stream. else, true.
     */
    virtual bool write(const char *key, const char *value, uint64_t timestamp) = 0;

    /**
     * flush output stream buffer
     */
    virtual void flush() = 0;

protected:

};

#endif
