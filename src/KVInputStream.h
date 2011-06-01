#ifndef KVINPUTSTREAM_H
#define KVINPUTSTREAM_H

#include "Global.h"

class KVInputStream {

public:

    /**
     * reset stream
     */
    virtual void reset() = 0;
    
    /**
     * get next <key, value> pair from stream
     * 
     * @return false if not <k,v> pair left. else, true.
     */
    virtual bool read(const char **key, const char **value) = 0;

protected:

};

#endif
