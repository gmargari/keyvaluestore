#ifndef KVTPRIORITYINPUTSTREAM_H
#define KVTPRIORITYINPUTSTREAM_H

#include "Global.h"
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
     * NOTE: the order of input streams in vector 'istreams' defines their priority,
     * in case for example two input streams have the same key. for example,
     * if both istreams[1] and istreams[4] have the same key 'k', the greatest
     * the <k,v> of istreams[1] will be considered greatest than that of istreams[4].
     *
     * So, when calling constructor, make sure to insert in the first position
     * of istreams the input stream from memstore (if any), since this has the
     * most recent value for a key, then the most recently written disk file etc.
     */
    KVTPriorityInputStream(vector<KVTInputStream *> istreams);

    /**
     * destructor
     */
    ~KVTPriorityInputStream();

    // inherited from KVTInputStream
    /**
     * reset priority queue
     */
    void reset();

    /**
     * get next <key, value, timestamp> tuple from priority queue
     *
     * @return false if no tuple left. else, true.
     */
    bool read(const char **key, const char **value, uint64_t *timestamp);

protected:

    typedef struct {
        const char *key;
        const char *value;
        uint64_t    timestamp;
        int sid;                // id of stream that we read this <k,v> from
    } heap_element;

    class heap_cmp {
    public:
        bool operator()(const heap_element *e1, const heap_element *e2) // returns true if e2 precedes e1
        {
            int cmp = strcmp(e1->key, e2->key);

            if (cmp > 0) {
                return true;
            } else if (cmp == 0 && e1->sid > e2->sid) {
//                 assert(e1->timestamp > e2->timestamp); // TODO: when disk timestamps are fixed, uncomment this
                return true;
            } else {
                return false;
            }
        }
    };

    void sanity_check();

    priority_queue<heap_element *, vector<heap_element *>, heap_cmp>   m_heap;
    vector<KVTInputStream *>      m_istreams;
    vector<heap_element *>        m_elements;
    // NOTE: heap contains 'heap_element *', not 'heap_element', to avoid copying the whole struct.
    // instead, we only copy the pointer to the element. this means we must be very carefull
    // not to insert again the same element, until it has been poped out from heap
};

#endif
