#include "Global.h"
#include "PriorityInputStream.h"

#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cstdlib>
#include <climits>

static const int SID_NOT_INITIALIZED = INT_MAX;

/*============================================================================
 *                            PriorityInputStream
 *============================================================================*/
PriorityInputStream::PriorityInputStream(vector<InputStream *> istreams)
    : m_pqueue(), m_istreams(istreams), m_elements(), m_last_sid(SID_NOT_INITIALIZED),
      m_start_key(NULL), m_start_keylen(0), m_end_key(NULL), m_end_keylen(0),
      m_start_incl(true), m_end_incl(true) {
    for (int i = 0; i < (int)m_istreams.size(); i++) {
        m_elements.push_back((pq_elem *)malloc(sizeof(pq_elem)));
        m_elements.back()->sid = i;
    }
}

/*============================================================================
 *                           ~PriorityInputStream
 *============================================================================*/
PriorityInputStream::~PriorityInputStream() {
    for (int i = 0; i < (int)m_elements.size(); i++) {
        free(m_elements[i]);
    }
}

/*============================================================================
 *                              set_key_range
 *============================================================================*/
void PriorityInputStream::set_key_range(const char *start_key, uint32_t start_keylen, const char *end_key, uint32_t end_keylen, bool start_incl, bool end_incl) {
    m_start_key = start_key;
    m_start_keylen = start_keylen;
    m_end_key = end_key;
    m_end_keylen = end_keylen;
    m_start_incl = start_incl;
    m_end_incl = end_incl;

    for (int i = 0; i < (int)m_istreams.size(); i++) {
        m_istreams[i]->set_key_range(m_start_key, m_start_keylen, m_end_key, m_end_keylen, m_start_incl, m_end_incl);
    }

    reset();
}

/*============================================================================
 *                              set_key_range
 *============================================================================*/
void PriorityInputStream::set_key_range(const char *start_key, uint32_t start_keylen, const char *end_key, uint32_t end_keylen) {
    set_key_range(start_key, start_keylen, end_key, end_keylen, true, false);
}

/*============================================================================
 *                                  reset
 *============================================================================*/
void PriorityInputStream::reset() {
    assert(sanity_check());

    // if priority queue non-empty, remove all elements
    while (!m_pqueue.empty()) {
        m_pqueue.pop();
    }

    // insert one element from each stream to priority queue
    for (int i = 0; i < (int)m_istreams.size(); i++) {
        if (m_istreams[i]->read(&(m_elements[i]->key), &(m_elements[i]->keylen),
                                &(m_elements[i]->value), &(m_elements[i]->valuelen), &(m_elements[i]->timestamp))) {
            m_pqueue.push(m_elements[i]);
        }
    }

    m_last_sid = -1;
    assert(sanity_check());
}

/*============================================================================
 *                                   read
 *============================================================================*/
bool PriorityInputStream::read(const char **key, uint32_t *keylen, const char **value, uint32_t *valuelen, uint64_t *timestamp) {
    pq_elem *top;

    assert(sanity_check());
    assert(key && value && timestamp);

    // m_last_sid: id of stream to which the last poped element belongs to
    if (m_last_sid == SID_NOT_INITIALIZED) {
        reset();
        assert(m_last_sid != SID_NOT_INITIALIZED);
    }

    if (m_last_sid != -1) {
        // the element poped from head last time belonged to stream 'm_last_sid'.
        // insert into priority queue a new element from that stream.
        assert(m_last_sid < (int)m_istreams.size());
        if (m_istreams[m_last_sid]->read(&(m_elements[m_last_sid]->key), &(m_elements[m_last_sid]->keylen),
                                         &(m_elements[m_last_sid]->value), &(m_elements[m_last_sid]->valuelen), &(m_elements[m_last_sid]->timestamp))) {
            m_pqueue.push(m_elements[m_last_sid]);
        }
    }

    if (m_pqueue.empty()) {
        return false;
    }

    // get top element
    top = m_pqueue.top();
    *key = top->key;
    *keylen = top->keylen;
    *value = top->value;
    *valuelen = top->valuelen;
    *timestamp = top->timestamp;
    m_last_sid = top->sid;
    m_pqueue.pop();

    assert(sanity_check());

    return true;
}

/*============================================================================
 *                                sanity_check
 *============================================================================*/
int PriorityInputStream::sanity_check() {
    int *x;
    vector<pq_elem *> tmp_vector;
    pq_elem *top;
    int num_streams;

    num_streams = m_istreams.size();
    x = (int *)malloc(num_streams * sizeof(int));
    memset(x, 0, num_streams*sizeof(int));
    assert(m_pqueue.size() <= (unsigned)num_streams); // always at most 'm_istreams.size()' elements in queue

    // get all elements of priority queue, check everything is ok
    while (!m_pqueue.empty()) {
        top = m_pqueue.top();
        assert(top->sid < num_streams);
        x[top->sid]++;
        assert(x[top->sid] <= 1);
        tmp_vector.push_back(top);
        m_pqueue.pop();
    }

    // reinsert elements to priority queue
    for (int i = 0; i < (int)tmp_vector.size(); i++) {
        m_pqueue.push(tmp_vector[i]);
    }
    tmp_vector.clear();

    free(x);

    return 1;
}
