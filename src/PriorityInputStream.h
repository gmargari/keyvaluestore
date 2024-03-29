// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_PRIORITYINPUTSTREAM_H_
#define SRC_PRIORITYINPUTSTREAM_H_

#include <string.h>
#include <vector>
#include <queue>

#include "./Global.h"
#include "./InputStream.h"
#include "./leveldb/slice.h"

using std::vector;
using std::priority_queue;

class PriorityInputStream: public InputStream {
  public:
    /**
     * constructor. each istream in 'istreams' must either be properly
     * positioned by caller (e.g. by calling set_key_range() for each istream),
     * or PriorityInputStream::set_key_range() must be called, before issuing
     * the first read() to priority input stream.
     */
    PriorityInputStream(vector<InputStream *> istreams);

    /**
     * destructor
     */
    ~PriorityInputStream();

    // inherited from InputStream
    void set_key_range(Slice start_key, Slice end_key, bool start_incl,
                       bool end_incl);
    void set_key_range(Slice start_key, Slice end_key);
    bool read(Slice *key, Slice *value, uint64_t *timestamp);

    // Undefined methods (just remove Weffc++ warning)
    PriorityInputStream(const PriorityInputStream&);
    PriorityInputStream& operator=(const PriorityInputStream&);

  private:
    typedef struct {
        Slice key;
        Slice value;
        uint64_t timestamp;
        int sid;                // id of stream from which we read this tuple
    } pq_elem;

    class pq_cmp {
      public:
        // returns true if e2 precedes e1
        bool operator()(const pq_elem *e1, const pq_elem *e2) {
            int cmp = strcmp(e1->key.data(), e2->key.data());

            if (cmp > 0) {
                return true;
            } else if (cmp == 0 && e2->timestamp > e1->timestamp) {
                return true;
            } else {
                return false;
            }
        }
    };

    void reset();

    int sanity_check();

    priority_queue<pq_elem *, vector<pq_elem *>, pq_cmp> m_pqueue;
    vector<InputStream *> m_istreams;
    vector<pq_elem *> m_elements;
    int   m_last_sid;  // id of stream last popped element belongs to
    Slice m_start_key;
    Slice m_end_key;
    bool  m_start_incl;
    bool  m_end_incl;
};

#endif  // SRC_PRIORITYINPUTSTREAM_H_
