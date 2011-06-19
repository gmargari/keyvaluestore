#ifndef KVTPRIORITYINPUTSTREAM_H
#define KVTPRIORITYINPUTSTREAM_H

#include "KVTInputStream.h"

#include <vector>
#include <queue>
#include <cstring>
#include <cassert>

using std::vector;
using std::priority_queue;

class KVTPriorityInputStream: public KVTInputStream {

public:

    /**
     * constructor
     */
    KVTPriorityInputStream(vector<KVTInputStream *> istreams);

    /**
     * destructor
     */
    ~KVTPriorityInputStream();

    // inherited from KVTInputStream
    void set_key_range(const char *start_key, const char *end_key, bool start_incl, bool end_incl);

    void set_key_range(const char *start_key, const char *end_key);

    bool read(const char **key, const char **value, uint64_t *timestamp);

    void reset();

protected:

    typedef struct {
        const char *key;
        const char *value;
        uint64_t    timestamp;
        int sid;                // id of stream from which we read this tuple
    } heap_element;

    class heap_cmp {
    public:
        bool operator()(const heap_element *e1, const heap_element *e2) // returns true if e2 precedes e1
        {
            int cmp = strcmp(e1->key, e2->key);

            if (cmp > 0) {
                return true;
            } else if (cmp == 0 && e2->timestamp > e1->timestamp) {
                return true;
            } else {
                return false;
            }
        }
    };

    int sanity_check();

    priority_queue<heap_element *, vector<heap_element *>, heap_cmp>   m_heap;
    vector<KVTInputStream *>      m_istreams;
    vector<heap_element *>        m_elements;
    int                           m_last_sid; // id of stream to which the last popped element belongs to
    // heap contains 'heap_element *', not 'heap_element', to avoid copying the whole struct.
    // instead, we only copy the pointer to the element. this means we must be very carefull
    // not to insert again the same element, until it has been poped out from heap
};

#endif
