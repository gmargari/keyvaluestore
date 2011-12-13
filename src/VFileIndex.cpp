// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#include "./VFileIndex.h"

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

/*============================================================================
 *                                VFileIndex
 *============================================================================*/
VFileIndex::VFileIndex()
    : m_map(), m_vfilesize(), m_stored_keys(0) {
}

/*============================================================================
 *                               ~VFileIndex
 *============================================================================*/
VFileIndex::~VFileIndex() {
    clear();
}

/*============================================================================
 *                                   add
 *============================================================================*/
void VFileIndex::add(Slice key, off_t offset) {
    char *cpkey;
    assert(sanity_check());

    cpkey = (char *)malloc(key.size() + 1);
    memcpy(cpkey, key.data(), key.size() + 1);
    assert(m_map.find(cpkey) == m_map.end());
    m_map[cpkey] = offset;

    assert(sanity_check());
}

/*============================================================================
 *                               set_vfilesize
 *============================================================================*/
void VFileIndex::set_vfilesize(off_t size) {
    m_vfilesize = size;
}

/*============================================================================
 *                            get_first_last_term
 *============================================================================*/
void VFileIndex::get_first_last_term(Slice *first, Slice *last) {
    TermOffsetMap::iterator iter;

    assert(first && last);
    iter = m_map.begin();
    *first = Slice(iter->first, strlen(iter->first));
    iter = m_map.end();
    --iter;
    *last = Slice(iter->first, strlen(iter->first));
}

/*============================================================================
 *                                  search
 *============================================================================*/
bool VFileIndex::search(Slice key, off_t *start_off, off_t *end_off) {
    TermOffsetMap::iterator iter;

    assert(sanity_check());
    assert(key.data() && start_off && end_off);
    assert(m_vfilesize && "index probably does not exist");

    iter = m_map.upper_bound(key.data());

    if (iter == m_map.end()) {  // no key greater than 'key'
        // if the last term of index is 'key'
        if (m_map.size() > 0 && (iter--, 1)
              && strcmp(iter->first, key.data()) == 0) {
            *start_off = iter->second;
            *end_off = m_vfilesize;
            assert(*start_off < *end_off);
            return true;
        } else {
            *start_off = -1;
            *end_off = -1;
            return false;
        }
    } else if (iter != m_map.begin()) {  // 'key' is between 'iter' and 'iter-1'
        *end_off = iter->second;
        --iter;
        *start_off = iter->second;
        assert(strcmp(key.data(), iter->first) >= 0 && (iter++, 1)
                 && strcmp(key.data(), iter->first) <= 0);
        assert(*start_off < *end_off);
        return true;
    } else {  // 'key' is smaller than first term in index
        *start_off = -1;
        *end_off = -1;
        return false;
    }
}

/*============================================================================
 *                                   clear
 *============================================================================*/
void VFileIndex::clear() {
    TermOffsetMap::iterator iter;

    assert(sanity_check());

    for (iter = m_map.begin(); iter != m_map.end(); ++iter) {
        assert(iter->first);
        free(const_cast<char *>(iter->first));
    }
    m_map.clear();
    m_vfilesize = 0;

    assert(sanity_check());
}

/*============================================================================
 *                            set_num_stored_leys
 *============================================================================*/
void VFileIndex::set_num_stored_leys(uint64_t numkeys) {
    m_stored_keys = numkeys;
}

/*============================================================================
 *                            get_num_stored_leys
 *============================================================================*/
uint64_t VFileIndex::get_num_stored_leys() {
    return m_stored_keys;
}

/*============================================================================
 *                                save_to_disk
 *============================================================================*/
void VFileIndex::save_to_disk(int fd) {
    size_t len;
    int num;
    TermOffsetMap::iterator iter;

    // write number of entries
    num = m_map.size();
    write(fd, &num, sizeof(num));

    // write the entries
    for (iter = m_map.begin(); iter != m_map.end(); ++iter) {
        len = strlen(iter->first);
        write(fd, &len, sizeof(len));
        write(fd, iter->first, len);
        write(fd, &(iter->second), sizeof(iter->second));
    }

    // write filesize & number of stored keys
    write(fd, &m_vfilesize, sizeof(m_vfilesize));
    write(fd, &m_stored_keys, sizeof(m_stored_keys));
}

/*============================================================================
 *                               load_from_disk
 *============================================================================*/
void VFileIndex::load_from_disk(int fd) {
    size_t len;
    int num;
    char key[MAX_INDEX_DIST];
    off_t offset;

    clear();

    // read number of entries
    num = m_map.size();
    read(fd, &num, sizeof(num));

    // read 'num' entries
    for (int i = 0; i < num; i++) {
        read(fd, &len, sizeof(len));
        read(fd, key, len);
        key[len] = '\0';
        read(fd, &offset, sizeof(offset));
        add(Slice(key, len), offset);
    }

    // read filesize & number of stored keys
    read(fd, &m_vfilesize, sizeof(m_vfilesize));
    read(fd, &m_stored_keys, sizeof(m_stored_keys));
}

/*============================================================================
 *                                sanity_check
 *============================================================================*/
int VFileIndex::sanity_check() {
    TermOffsetMap::iterator iter;
    const char *prev_key = NULL;
    off_t prev_offs = -1;

    assert(m_map.size() == 0 || (m_map.begin())->second == 0);
    for (iter = m_map.begin(); iter != m_map.end(); ++iter) {
        assert(prev_key == NULL || strcmp(iter->first, prev_key) > 0);
        assert(prev_offs == -1  || iter->second > prev_offs);
        assert(prev_offs != -1  || iter->second - prev_offs <= MAX_INDEX_DIST);
        prev_key = iter->first;
        prev_offs = iter->second;
    }

    return 1;
}

