#include "Global.h"
#include "KVPriorityInputStream.h"
#include "KVInputStream.h"

#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cstdlib>

/*========================================================================
 *                           KVPriorityInputStream
 *========================================================================*/
KVPriorityInputStream::KVPriorityInputStream(std::vector<KVInputStream *> istreams)
{
    m_istreams = istreams;
    for (int i = 0; i < (int)m_istreams.size(); i++) {
        m_elements.push_back((heap_element *)malloc(sizeof(heap_element)));
        m_elements.back()->sid = i;
    }
    
    reset();
}

/*========================================================================
 *                          ~KVPriorityInputStream
 *========================================================================*/
KVPriorityInputStream::~KVPriorityInputStream()
{
    for (int i = 0; i < (int)m_elements.size(); i++) {
        free(m_elements[i]);
    }
}

/*========================================================================
 *                                 reset
 *========================================================================*/
void KVPriorityInputStream::reset()
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
bool KVPriorityInputStream::read(const char **key, const char **value)
{
    // NOTE: if after merging a value is greater than MAX_KVSIZE --> error!
    //     assert(strlen(key) + 1 <= MAX_KVSIZE);
    
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
void KVPriorityInputStream::sanity_check()
{
    vector<int> x(m_heap.size(), 0);
    vector<heap_element *> tmp_vector;
    heap_element *top;
    int heap_len;

return; // TODO: MPOREI NA XALAW TO SORO!
    
#if DBGLVL < 2 // TODO: replace this code with something like (return_if_dbglvl_lt_2())
    return;
#endif

    heap_len = m_heap.size();
    assert(m_heap.size() <= m_istreams.size()); // always at most 'm_istreams.size()' elements in queue

    // get all elements of heap
    while (!m_heap.empty()) {
        top = m_heap.top();
        x[top->sid]++;
        tmp_vector.push_back(top);
        m_heap.pop();
    }

    // assert we only have one element from each stream in heap
    for (int i = 0; i < heap_len; i++) {
        assert(x[i] <= 1);
    }

    // reinsert elements to heap
    for (int i = 0; i < (int)tmp_vector.size(); i++) {
        m_heap.push(tmp_vector[i]);
    }
    tmp_vector.clear();
}

#include <cstdio>

/*=======================================================================*
 *                              print_heap
 *=======================================================================*/
void KVPriorityInputStream::print_heap()
{
    vector<heap_element *> tmp_vector;
    heap_element *top;

    printf("heap:\n");
    while (!m_heap.empty()) {
        top = m_heap.top();
        printf("[%s][%s][%d]\n", top->key, top->value, top->sid);
        tmp_vector.push_back(top);
        m_heap.pop();
    }

    for (int i = 0; i < (int)tmp_vector.size(); i++)
        m_heap.push(tmp_vector[i]);
    tmp_vector.clear();
}
