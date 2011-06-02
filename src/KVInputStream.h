#ifndef KVINPUTSTREAM_H
#define KVINPUTSTREAM_H

#include "Global.h"

class KVInputStream {

public:

    /**
     * constructor
     */
    KVInputStream();

    /**
     * destructor (define as 'virtual' so it will call destructors of derived classes)
     */
    virtual ~KVInputStream();  
    
    /**
     * reset stream
     */
    virtual void reset() = 0;
    
    /**
     * get next <key, value> pair from stream
     * 
     * @return false if no <k,v> pair left. else, true.
     */
    virtual bool read(const char **key, const char **value) = 0;

protected:

};

#endif
