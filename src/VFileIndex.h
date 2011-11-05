#ifndef VFILEINDEX_H
#define VFILEINDEX_H

#include <cstdio>
#include <cstring>
#include <map>

using std::map;

// TODO: since entries are appended in index, we could use a container that
// has O(1) inserts, such as vectors, and use binary search to find a specific
// term. But as long as insertions are rare, while searches are frequent, maybe
// the map container is ok.

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
     * add (append) a <term, offset> entry in index
     */
    void add(const char *term, off_t offset);

    /**
     * set the vfile disk size (used to determine where the last entry ends,
     * see search() function)
     */
    void set_vfilesize(off_t size);

    /**
     * return first and last term in index
     */
    void get_first_last_term(const char **first, const char **last);

    /**
     * find the location on disk where the value corresponding to key 'term'
     * might have been stored. normally, after this call we would read all
     * bytes stored on vfile between 'start_off' and 'end_off' and linearly
     * search for a <'term', value> pair.
     *
     * @param term (in) term to search for
     * @param start_off (out) start offset on disk file
     * @param end_off (out) end offset on disk file
     */
    bool search(const char *term, off_t *start_off, off_t *end_off);

    /**
     * clear entries of index
     */
    void clear();

    /**
     * save index to file
     */
    void save_to_disk(int fd);

    /**
     * load index from file
     */
    void load_from_disk(int fd);

    struct cmp_str { // required, in order for the map to have its keys sorted
        bool operator()(char const *a, char const *b) {
            return strcmp(a, b) < 0;
        }
    };

    typedef map<const char *, off_t, cmp_str> TermOffsetMap;

protected:

    int sanity_check();

    TermOffsetMap m_map;          // index containing <term, offset> pairs
    off_t         m_vfilesize;    // size of vfile on disk
};

#endif
