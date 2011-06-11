#include "Global.h"
#include "VFileIndex.h"

#include <cassert>
#include <cstdlib>

/*========================================================================
 *                            VFileIndex
 *========================================================================*/
VFileIndex::VFileIndex()
{

}

/*========================================================================
 *                           ~VFileIndex
 *========================================================================*/
VFileIndex::~VFileIndex()
{
    clear();
}

/*========================================================================
 *                                 add
 *========================================================================*/
void VFileIndex::add(const char *key, off_t offset)
{
    char *cpkey;
    sanity_check();

    cpkey = strdup(key);
    assert(m_map.find(cpkey) == m_map.end());
    m_map[cpkey] = offset;

    sanity_check();
}

/*========================================================================
 *                            set_vfilesize
 *========================================================================*/
void VFileIndex::set_vfilesize(off_t size)
{
    m_vfilesize = size;
}

/*========================================================================
 *                                search
 *========================================================================*/
bool VFileIndex::search(const char *key, off_t *start_off, off_t *end_off)
{
    termoffsmap::iterator iter;

    sanity_check();
    assert(key && start_off && end_off);
    assert(m_vfilesize);

    iter = m_map.upper_bound(key);

    // if no key greater than 'key'
    if (iter == m_map.end()) {
        // if the last term of index is 'key'
        if (m_map.size() > 0 && (iter--, 1) && strcmp(iter->first, key) == 0) {
            *start_off = iter->second;
            *end_off = m_vfilesize;
            assert(*start_off < *end_off);
            return true;
        } else {
            *start_off = -1;
            *end_off = -1;
            return false;
        }
    }
    // else, if 'key' is between 'iter' and 'iter - 1'
    else if (iter != m_map.begin()) {
        *end_off = iter->second;
        iter--;
        *start_off = iter->second;
        assert(strcmp(key, iter->first) >= 0 && (iter++, 1) && strcmp(key, iter->first) <= 0);
        assert(*start_off < *end_off);
        return true;
    }
    // else, 'key' is smaller than first term in index
    else {
        *start_off = -1;
        *end_off = -1;
        return false;
    }
}

/*========================================================================
 *                                clear
 *========================================================================*/
void VFileIndex::clear()
{
    termoffsmap::iterator iter;

    sanity_check();

    for (iter = m_map.begin(); iter != m_map.end(); iter++) {
        assert(iter->first);
        free(const_cast<char *>(iter->first));
    }
    m_map.clear();
    m_vfilesize = 0;

    sanity_check();
}

/*=======================================================================*
 *                              sanity_check
 *=======================================================================*/
void VFileIndex::sanity_check()
{
    termoffsmap::iterator iter;
    const char *prev_key = NULL;
    off_t prev_offs = -1;

    return_if_dbglvl_lt_2();

    assert(m_map.size() == 0 || (m_map.begin())->second == 0);
    for (iter = m_map.begin(); iter != m_map.end(); iter++) {
        assert(prev_key == NULL || strcmp(iter->first, prev_key) > 0);
        assert(prev_offs == -1  || iter->second > prev_offs);
        assert(prev_offs != -1  || iter->second - prev_offs <= MAX_INDEX_DIST);
        prev_key = iter->first;
        prev_offs = iter->second;
    }
}