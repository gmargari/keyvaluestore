// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_MAPINPUTSTREAM_H_
#define SRC_MAPINPUTSTREAM_H_

#include "./Global.h"
#include "./InputStream.h"
#include "./Map.h"

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
    void set_key_range(const char *start_key, uint32_t start_keylen, const char *end_key, uint32_t end_keylen, bool start_incl, bool end_incl);
    void set_key_range(const char *start_key, uint32_t start_keylen, const char *end_key, uint32_t end_keylen);
    bool read(const char **key, uint32_t *keylen, const char **value, uint32_t *valuelen, uint64_t *timestamp);

    // Undefined methods (just remove Weffc++ warning)
    MapInputStream(const MapInputStream&);
    MapInputStream& operator=(const MapInputStream&);

  protected:
    Map                    *m_map;
    Map::KVTMap::iterator   m_iter;
    Map::KVTMap::iterator   m_iter_end;
    const char             *m_start_key;
    uint32_t                m_start_keylen;
    const char             *m_end_key;
    uint32_t                m_end_keylen;
    bool                    m_start_incl;
    bool                    m_end_incl;
};

#endif  // SRC_MAPINPUTSTREAM_H_
