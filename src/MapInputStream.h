// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_MAPINPUTSTREAM_H_
#define SRC_MAPINPUTSTREAM_H_

#include "./Global.h"
#include "./InputStream.h"
#include "./Map.h"
#include "./Slice.h"

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
    void set_key_range(Slice start_key, Slice end_key, bool start_incl, bool end_incl);
    void set_key_range(Slice start_key, Slice end_key);
    bool read(Slice *key, Slice *value, uint64_t *timestamp);

    // Undefined methods (just remove Weffc++ warning)
    MapInputStream(const MapInputStream&);
    MapInputStream& operator=(const MapInputStream&);

  private:
    Map                    *m_map;
    Map::KVTMap::iterator   m_iter;
    Map::KVTMap::iterator   m_iter_end;
    Slice                   m_start_key;
    Slice                   m_end_key;
    bool                    m_start_incl;
    bool                    m_end_incl;
};

#endif  // SRC_MAPINPUTSTREAM_H_
