#ifndef KVMAPINPUTSTREAM_H
#define KVMAPINPUTSTREAM_H

#include "Global.h"
#include "KVInputStream.h"
#include "KVMap.h"

class KVMapInputStream: public KVInputStream {

public:

    /**
     * constructor
     */
    KVMapInputStream(KVMap *kvmap);

    /**
     * destructor
     */
    ~KVMapInputStream();

    // inherited from KVInputStream
    /**
     * reset stream
     */    
    void reset();

    /**
     * get next <key, value> pair from stream
     *
     * @return false if no <k,v> pair left. else, true.
     */
    bool read(const char **key, const char **value);

protected:

    void sanity_check();
    
    KVMap                       *m_kvmap;
    KVMap::kvmap::iterator       iter;

};

#endif
