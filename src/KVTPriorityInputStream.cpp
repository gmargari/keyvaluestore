#include "Global.h"
#include "KVTPriorityInputStream.h"
#include "KVTInputStream.h"

#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cstdlib>

/*========================================================================
 *                           KVTPriorityInputStream
 *========================================================================*/
KVTPriorityInputStream::KVTPriorityInputStream(vector<KVTInputStream *> istreams)
{
    m_istreams = istreams;
    for (int i = 0; i < (int)m_istreams.size(); i++) {
        m_elements.push_back((heap_element *)malloc(sizeof(heap_element)));
        m_elements.back()->sid = i;
    }

    reset();
}

/*========================================================================
 *                          ~KVTPriorityInputStream
 *========================================================================*/
KVTPriorityInputStream::~KVTPriorityInputStream()
{
    for (int i = 0; i < (int)m_elements.size(); i++) {
        free(m_elements[i]);
    }
}

/*========================================================================
 *                                 reset
 *========================================================================*/
void KVTPriorityInputStream::reset()
{
    sanity_check();

    // if heap non-empty, remove all elements
    while (!m_heap.empty()) {
        m_heap.pop();
    }

    // reset all streams, insert one element from each stream to heap
    for (int i = 0; i < (int)m_istreams.size(); i++) {
        m_istreams[i]->reset();
        if (m_istreams[i]->read(&(m_elements[i]->key), &(m_elements[i]->value))) {
            m_heap.push(m_elements[i]);
        }
    }
    sanity_check();
}

/*========================================================================
 *                                 read
 *========================================================================*/
bool KVTPriorityInputStream::read(const char **key, const char **value)
{
    // NOTE: if after merging a value is greater than MAX_KVTSIZE --> error!
    //     assert(strlen(key) + 1 <= MAX_KVTSIZE);

    if (m_heap.empty()) {
        return false;
    } else {
        heap_element *top;
        int sid;

        sanity_check();

        // get top element
        top = m_heap.top();
        *key = top->key;
        *value = top->value;
        sid = top->sid;

        m_heap.pop();

        // 'top' elements belongs to stream 'sid'. forward that stream and
        // insert into heap a new element from this stream
        if (m_istreams[sid]->read(&(m_elements[sid]->key), &(m_elements[sid]->value))) {
            m_heap.push(m_elements[sid]);
        }

        sanity_check();

        return true;
    }
}

/*=======================================================================*
 *                              sanity_check
 *=======================================================================*/
void KVTPriorityInputStream::sanity_check()
{
    int *x;
    vector<heap_element *> tmp_vector;
    heap_element *top;
    int num_streams;

    return_if_dbglvl_lt_2();

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
}
