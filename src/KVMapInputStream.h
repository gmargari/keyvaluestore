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
     * constructor of an input stream that returns all memory <k,v> pairs with 
     * keys between 'start_key' (inclusive) and 'end_key' (exclusive), 
     * i.e. start_key <= k < end_key
     */
    KVMapInputStream(KVMap *kvmap, const char *start_key, const char *end_key);

    /**
     * as above, but user can define whether 'start_key' and 'end_key' are
     * inclusive or not
     */
    KVMapInputStream(KVMap *kvmap, const char *start_key, const char *end_key, bool start_incl, bool end_incl);
    
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

    void init(KVMap *kvmap, const char *start_key, const char *end_key, bool start_incl, bool end_incl);
    void sanity_check();
    
    KVMap                       *m_kvmap;
    KVMap::kvmap::iterator       m_iter;
    KVMap::kvmap::iterator       m_iter_end;
    char *                       m_start_key;
    char *                       m_end_key;
    bool                         m_start_incl;
    bool                         m_end_incl;
};

#endif
