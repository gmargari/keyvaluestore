#ifndef KVOUTPUTSTREAM_H
#define KVOUTPUTSTREAM_H

#include "Global.h"

class KVOutputStream {

public:

    /**
     * reset stream
     */
    virtual void reset() = 0;

    /**
     * write <key, value> to stream
     *
     * @return false if could not write <k,v> to stream. else, true.
     */
    virtual bool write(const char *key, const char *value) = 0;

    /**
     * flush output stream buffer
     */
    virtual void flush() = 0;

protected:

};

#endif
