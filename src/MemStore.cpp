// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#include "./Global.h"
#include "./MemStore.h"

#include <assert.h>

#include "./MapInputStream.h"

using std::make_pair;

/*============================================================================
 *                                 MemStore
 *============================================================================*/
MemStore::MemStore()
    : m_map(), m_maxsize(DEFAULT_MEMSTORE_SIZE), m_num_keys(0), m_size(0),
      m_size_when_serialized(0) {
    add_map(Slice("", 0));
}

/*============================================================================
 *                                ~MemStore
 *============================================================================*/
MemStore::~MemStore() {
    for (int i = 0; i < (int)m_map.size(); i++) {
        delete m_map[i].map;
    }
}

/*============================================================================
 *                               set_maxsize
 *============================================================================*/
void MemStore::set_maxsize(uint64_t maxsize) {
    m_maxsize = maxsize;
}

/*============================================================================
 *                               get_maxsize
 *============================================================================*/
uint64_t MemStore::get_maxsize() {
    return m_maxsize;
}

/*============================================================================
 *                                   put
 *============================================================================*/
bool MemStore::put(Slice key, Slice value, uint64_t timestamp) {
    bool ret;
    Map *map = get_map(key);

    m_num_keys -= map->get_num_keys();
    m_size -= map->get_size();
    m_size_when_serialized -= map->get_size_when_serialized();
    ret = map->put(key, value, timestamp);
    m_num_keys += map->get_num_keys();
    m_size += map->get_size();
    m_size_when_serialized += map->get_size_when_serialized();

    return ret;
}

/*============================================================================
 *                                   get
 *============================================================================*/
bool MemStore::get(Slice key, Slice *value, uint64_t *timestamp) {
    char *valuecp;
    if (get_map(key)->get(key, value, timestamp)) {
        valuecp = (char *)malloc(value->size() + 1);
        memcpy(valuecp, value->data(), value->size() + 1);  // copy value
        *value = Slice(valuecp, value->size());
        return true;
    } else {
        return false;
    }
}

/*============================================================================
 *                              get_num_keys
 *============================================================================*/
uint64_t MemStore::get_num_keys() {
    return m_num_keys;
}

/*============================================================================
 *                                get_size
 *============================================================================*/
uint64_t MemStore::get_size() {
    return m_size;
}

/*============================================================================
 *                           get_size_when_serialized
 *============================================================================*/
uint64_t MemStore::get_size_when_serialized() {
    return m_size_when_serialized;
}

/*============================================================================
 *                                  will_fill
 *============================================================================*/
bool MemStore::will_fill(Slice key, Slice value, uint64_t timestamp) {
    return (m_size + Map::kv_size(key, value, timestamp) > m_maxsize);
}

/*============================================================================
 *                                   clear
 *============================================================================*/
void MemStore::clear_map() {
    assert(m_map.size() == 1);
    clear_map(m_map[0].map);
}

/*============================================================================
 *                             new_map_inputstream
 *============================================================================*/
MapInputStream *MemStore::new_map_inputstream() {
    assert(m_map.size() == 1);
    return new MapInputStream(m_map[0].map);
}

/*============================================================================
 *                             new_map_inputstream
 *============================================================================*/
MapInputStream *MemStore::new_map_inputstream(Slice key) {
    return new MapInputStream(m_map[idx_of_map(key)].map);
}

/*============================================================================
 *                                 add_map
 *============================================================================*/
void MemStore::add_map(Slice key) {
    int pos = idx_of_map(key);
    MapEntry newentry;

    if (m_map.size() > 0 && strcmp(m_map[pos].key, key.data()) == 0) {
        return;  // map with same key already exists
    }

    memcpy(newentry.key, key.data(), key.size() + 1);
    newentry.map = new Map();

    if (m_map.size()) {
        pos++;
    }
    m_map.insert(m_map.begin() + pos, newentry);

    sanity_check();
}

/*============================================================================
 *                                 get_map
 *============================================================================*/
Map *MemStore::get_map(Slice key) {
    return m_map[idx_of_map(key)].map;
}

/*============================================================================
 *                                 clear_map
 *============================================================================*/
void MemStore::clear_map(Slice key) {
    clear_map(get_map(key));
}

/*============================================================================
 *                                 clear_map
 *============================================================================*/
void MemStore::clear_map(Map *map) {
    m_num_keys -= map->get_num_keys();
    m_size -= map->get_size();
    m_size_when_serialized -= map->get_size_when_serialized();
    map->clear();
}

/*============================================================================
 *                                 idx_of_map
 *============================================================================*/
int MemStore::idx_of_map(Slice key) {
    int first, last, mid, cmp;

    sanity_check();

    if (m_map.size() == 0) {
        return 0;
    }

    first = 0;
    last = m_map.size() - 1;
    while (first <= last) {
        mid = (last - first)/2 + first;  // avoid overflow
        cmp = strcmp(key.data(), m_map[mid].key);
        if (cmp > 0) {
            first = mid + 1;
        } else if (cmp < 0) {
            last = mid - 1;
        } else {
            break;
        }
    }

    if (strcmp(key.data(), m_map[mid].key) < 0) {
        mid--;
    }

    // assert 'key' belongs to range [map[mid].key, map[mid+1].key)
    assert(strcmp(key.data(), m_map[mid].key) >= 0 &&
             (mid == (int)m_map.size() - 1 ||
               strcmp(key.data(), m_map[mid + 1].key) < 0));
    assert(mid >= 0 && mid < (int)m_map.size());

    return mid;
}

/*============================================================================
 *                                sanity_check
 *============================================================================*/
int MemStore::sanity_check() {
#if DBGLVL > 1
    for (int i = 0; i < (int)m_map.size() - 1; i++) {
        assert(strcmp(m_map[i].key, m_map[i+1].key) < 0);
    }
#endif

    return 1;
}
