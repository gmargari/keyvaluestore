#ifndef KVTMAPINPUTSTREAM_H
#define KVTMAPINPUTSTREAM_H

#include "Global.h"
#include "KVTInputStream.h"
#include "KVTMap.h"

class KVTMapInputStream: public KVTInputStream {

public:

    /**
     * constructor
     */
    KVTMapInputStream(KVTMap *kvtmap);

    /**
     * constructor of an input stream that returns all memory <k,v> pairs with
     * keys between 'start_key' (inclusive) and 'end_key' (exclusive),
     * i.e. start_key <= k < end_key
     */
    KVTMapInputStream(KVTMap *kvtmap, const char *start_key, const char *end_key);

    /**
     * as above, but user can define whether 'start_key' and 'end_key' are
     * inclusive or not
     */
    KVTMapInputStream(KVTMap *kvtmap, const char *start_key, const char *end_key, bool start_incl, bool end_incl);

    /**
     * destructor
     */
    ~KVTMapInputStream();

    // inherited from KVTInputStream
    /**
     * reset stream
     */
    void reset();

    /**
     * get next <key, value, timestamp> tuple from stream
     * (pointers are valid only until next call to function. if caller wants to
     * use them after next call, he must copy key and value)
     *
     * @return false if no tuple left. else, true.
     */
    bool read(const char **key, const char **value, uint64_t *timestamp);

protected:

    void init(KVTMap *kvtmap, const char *start_key, const char *end_key, bool start_incl, bool end_incl);
    void sanity_check();

    KVTMap                      *m_kvtmap;
    KVTMap::kvtmap::iterator     m_iter;
    KVTMap::kvtmap::iterator     m_iter_end;
    char *                       m_start_key;
    char *                       m_end_key;
    bool                         m_start_incl;
    bool                         m_end_incl;
};

#endif
