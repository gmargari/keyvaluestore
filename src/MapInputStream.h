#ifndef KVTMAPINPUTSTREAM_H
#define KVTMAPINPUTSTREAM_H

#include "InputStream.h"
#include "Map.h"

class MapInputStream: public InputStream {

public:

    /**
     * constructor
     */
    MapInputStream(Map *map);

    /**
     * destructor
     */
    ~MapInputStream();

    // inherited from InputStream
    void set_key_range(const char *start_key, const char *end_key, bool start_incl, bool end_incl);

    void set_key_range(const char *start_key, const char *end_key);

    bool read(const char **key, const char **value, uint64_t *timestamp);

    void reset();

protected:

    void init(Map *map, const char *start_key, const char *end_key, bool start_incl, bool end_incl);
    int sanity_check();

    Map                      *m_map;
    Map::KVTMap::iterator     m_iter;
    Map::KVTMap::iterator     m_iter_end;
    const char                  *m_start_key;
    const char                  *m_end_key;
    bool                         m_start_incl;
    bool                         m_end_incl;
};

#endif
