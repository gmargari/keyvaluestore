#include "Global.h"
#include "PriorityInputStream.h"

#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cstdlib>

/*============================================================================
 *                            PriorityInputStream
 *============================================================================*/
PriorityInputStream::PriorityInputStream(vector<InputStream *> istreams)
{
    m_istreams = istreams;
    for (int i = 0; i < (int)m_istreams.size(); i++) {
        m_elements.push_back((heap_element *)malloc(sizeof(heap_element)));
        m_elements.back()->sid = i;
    }

    reset();
}

/*============================================================================
 *                           ~PriorityInputStream
 *============================================================================*/
PriorityInputStream::~PriorityInputStream()
{
    for (int i = 0; i < (int)m_elements.size(); i++) {
        free(m_elements[i]);
    }
}

/*============================================================================
 *                              set_key_range
 *============================================================================*/
void PriorityInputStream::set_key_range(const char *start_key, const char *end_key, bool start_incl, bool end_incl)
{
    for (int i = 0; i < (int)m_istreams.size(); i++) {
        m_istreams[i]->set_key_range(start_key, end_key, start_incl, end_incl);
    }
}

/*============================================================================
 *                              set_key_range
 *============================================================================*/
void PriorityInputStream::set_key_range(const char *start_key, const char *end_key)
{
    set_key_range(start_key, end_key, true, false);
}

/*============================================================================
 *                                  reset
 *============================================================================*/
void PriorityInputStream::reset()
{
    assert(sanity_check());

    // if heap non-empty, remove all elements
    while (!m_heap.empty()) {
        m_heap.pop();
    }

    // reset all streams, insert one element from each stream to heap
    for (int i = 0; i < (int)m_istreams.size(); i++) {
        m_istreams[i]->reset();
        if (m_istreams[i]->read(&(m_elements[i]->key), &(m_elements[i]->value), &(m_elements[i]->timestamp))) {
            m_heap.push(m_elements[i]);
        }
    }

    m_last_sid = -1;
    assert(sanity_check());
}

/*============================================================================
 *                                   read
 *============================================================================*/
bool PriorityInputStream::read(const char **key, const char **value, uint64_t *timestamp)
{
    heap_element *top;

    assert(sanity_check());
    assert(key && value && timestamp);

    // m_last_sid: id of stream to which the last poped element belongs to
    if (m_last_sid != -1) {
        // the element poped from head last time belonged to stream 'm_last_sid'.
        // insert into heap a new element from that stream.
        if (m_istreams[m_last_sid]->read(&(m_elements[m_last_sid]->key), &(m_elements[m_last_sid]->value), &(m_elements[m_last_sid]->timestamp))) {
            m_heap.push(m_elements[m_last_sid]);
        }
    }

    if (m_heap.empty()) {
        return false;
    }

    // get top element
    top = m_heap.top();
    *key = top->key;
    *value = top->value;
    *timestamp = top->timestamp;
    m_last_sid = top->sid;
    m_heap.pop();

    assert(sanity_check());

    return true;
}

/*============================================================================
 *                                sanity_check
 *============================================================================*/
int PriorityInputStream::sanity_check()
{
    int *x;
    vector<heap_element *> tmp_vector;
    heap_element *top;
    int num_streams;

    num_streams = m_istreams.size();
    x = (int *)malloc(num_streams * sizeof(int));
    memset(x, 0, num_streams*sizeof(int));
    assert(m_heap.size() <= (unsigned)num_streams); // always at most 'm_istreams.size()' elements in queue

    // get all elements of heap, check everything is ok
    while (!m_heap.empty()) {
        top = m_heap.top();
        assert(top->sid < num_streams);
        x[top->sid]++;
        assert(x[top->sid] <= 1);
        tmp_vector.push_back(top);
        m_heap.pop();
    }

    // reinsert elements to heap
    for (int i = 0; i < (int)tmp_vector.size(); i++) {
        m_heap.push(tmp_vector[i]);
    }
    tmp_vector.clear();

    free(x);

    return 1;
}
