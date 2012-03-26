// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_VFILEINDEX_H_
#define SRC_VFILEINDEX_H_

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <map>

#include "./Global.h"
#include "./leveldb/slice.h"

using std::map;

class VFileIndex {
  public:
    /**
     * constructor
     */
    VFileIndex();

    /**
     * destructor
     */
    ~VFileIndex();

    /**
     * add (append) a <key, offset> entry in index
     */
    void add(Slice key, off_t offset);

    /**
     * set the vfile disk size (used to determine where the last entry ends,
     * see search() function)
     */
    void set_vfilesize(off_t size);

    /**
     * return first and last term in index
     */
    void get_first_last_term(Slice *first, Slice *last);

    /**
     * find the location on disk where the value corresponding to key 'term'
     * might have been stored. normally, after this call we would read all
     * bytes stored on vfile between 'start_off' and 'end_off' and linearly
     * search for a <'term', value> pair.
     *
     * @param key (in) term to search for
     * @param start_off (out) start offset on disk file
     * @param end_off (out) end offset on disk file
     */
    bool search(Slice key, off_t *start_off, off_t *end_off);

    /**
     * clear entries of index
     */
    void clear();

    /**
     * set/get the number of keys stored in disk file
     */
    void     set_num_stored_leys(uint64_t numkeys);
    uint64_t get_num_stored_leys();

    /**
     * save index to file
     */
    void save_to_disk(int fd);

    /**
     * load index from file
     */
    void load_from_disk(int fd);

    struct cmp_str {  // required in order for the map to have its keys sorted
        bool operator()(char const *a, char const *b) {
            return strcmp(a, b) < 0;
        }
    };

    typedef map<const char *, off_t, cmp_str> TermOffsetMap;

  private:
    int sanity_check();

    TermOffsetMap m_map;          // index containing <term, offset> pairs
    off_t         m_vfilesize;    // size of vfile on disk
    uint64_t      m_stored_keys;  // number of keys stored in vfile
};

#endif  // SRC_VFILEINDEX_H_
