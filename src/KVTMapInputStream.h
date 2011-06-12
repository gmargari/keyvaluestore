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
     * destructor
     */
    ~KVTMapInputStream();

    // inherited from KVTInputStream
    void set_key_range(const char *start_key, const char *end_key, bool start_incl, bool end_incl);

    void set_key_range(const char *start_key, const char *end_key);

    bool read(const char **key, const char **value, uint64_t *timestamp);

    void reset();

protected:

    void init(KVTMap *kvtmap, const char *start_key, const char *end_key, bool start_incl, bool end_incl);
    void sanity_check();

    KVTMap                      *m_kvtmap;
    KVTMap::kvtmap::iterator     m_iter;
    KVTMap::kvtmap::iterator     m_iter_end;
    char                        *m_start_key;
    char                        *m_end_key;
    bool                         m_start_incl;
    bool                         m_end_incl;
};

#endif
