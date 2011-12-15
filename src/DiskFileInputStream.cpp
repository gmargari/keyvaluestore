// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#include "./DiskFileInputStream.h"

#include <string.h>
#include <assert.h>

#include "./DiskFile.h"
#include "./Buffer.h"

/*============================================================================
 *                             DiskFileInputStream
 *============================================================================*/
DiskFileInputStream::DiskFileInputStream(DiskFile *file, uint32_t bufsize)
    : m_diskfile(file), m_offs(0), m_buf(NULL), m_start_key(NULL), m_start_keylen(0),
      m_end_key(NULL), m_end_keylen(0), m_start_incl(true), m_end_incl(true) {
    m_buf = new Buffer(bufsize);
    set_key_range(NULL, 0, NULL, 0);
}

/*============================================================================
 *                             DiskFileInputStream
 *============================================================================*/
DiskFileInputStream::DiskFileInputStream(DiskFile *file, char *buf, uint32_t bufsize)
    : m_diskfile(file), m_offs(0), m_buf(NULL), m_start_key(NULL), m_start_keylen(0),
      m_end_key(NULL), m_end_keylen(0), m_start_incl(true), m_end_incl(true) {
    m_buf = new Buffer(buf, bufsize);
}

/*============================================================================
 *                            ~DiskFileInputStream
 *============================================================================*/
DiskFileInputStream::~DiskFileInputStream() {
    delete m_buf;
}

/*============================================================================
 *                                set_key_range
 *============================================================================*/
void DiskFileInputStream::set_key_range(const char *start_key, uint32_t start_keylen, const char *end_key, uint32_t end_keylen, bool start_incl, bool end_incl) {
    m_start_key = start_key;
    m_start_keylen = start_keylen;
    m_end_key = end_key;
    m_end_keylen = end_keylen;
    m_start_incl = start_incl;
    m_end_incl = end_incl;

    m_buf->clear();
}

/*============================================================================
 *                              set_key_range
 *============================================================================*/
void DiskFileInputStream::set_key_range(const char *start_key, uint32_t start_keylen, const char *end_key, uint32_t end_keylen) {
    set_key_range(start_key, start_keylen, end_key, end_keylen, true, false);
}

/*============================================================================
 *                                  read
 *============================================================================*/
bool DiskFileInputStream::read(const char **key, uint32_t *keylen, const char **value, uint32_t *valuelen, uint64_t *timestamp) {
    int cmp;
    off_t off1, off2;
    const char *tmpkey, *tmpvalue;
    uint32_t tmpkeylen, tmpvaluelen;
    uint64_t tmpts;
    bool ret;

    *key = NULL;
    *value = NULL;
    *timestamp = 0;

    //--------------------------------------------------------------------------
    // this code is executed only the first time read() is called, after reset()
    // or set_key_range(): seek to the first valid key on disk
    //--------------------------------------------------------------------------
    if (m_buf->size() == 0) {

        assert(m_buf->used() == 0);

        // if caller defined a startkey, seek to the first valid key on disk
        if (m_start_key) {

            // if m_start_key was stored on disk it would be between off1 & off2
            ret = m_diskfile->search(m_start_key, m_start_keylen, &off1, &off2);
            if (ret == false) {
                return false;
            }

            // read in buffer all bytes between 'off1' and 'off2'
            m_buf->clear();
            m_diskfile->fill(m_buf, off2 - off1, off1);
            m_offs = off2;
            assert(m_buf->size() == off2 - off1);

            // check all buffer tuples until we find a key >= startkey
            while (m_buf->deserialize(&tmpkey, &tmpkeylen, &tmpvalue, &tmpvaluelen, &tmpts, false)) {
                if ((cmp = strcmp(tmpkey, m_start_key)) >= 0) {
                    break;
                }
            }

            if (cmp > 0 || (cmp == 0 && m_start_incl == true)) {
                // must seek back at beginning of key tuple
                m_buf->undo_deserialize(tmpkey, tmpkeylen, tmpvalue, tmpvaluelen, tmpts);
            }
        }
        // else, seek to the first key on disk
        else {
            m_offs = 0;
            m_offs += m_diskfile->fill(m_buf, m_offs);
        }
    }

    //--------------------------------------------------------------------------
    // read next <key,value,timestamp> from file
    //--------------------------------------------------------------------------
    if ( ! m_buf->deserialize(&tmpkey, &tmpkeylen, &tmpvalue, &tmpvaluelen, &tmpts, false)) {

        // maybe we need to read more bytes in buffer to deserialize tuple
        m_buf->keep_unused();
        m_offs += m_diskfile->fill(m_buf, m_offs);

        // this should now work, unless there are no bytes left in file
        if ( ! m_buf->deserialize(&tmpkey, &tmpkeylen, &tmpvalue, &tmpvaluelen, &tmpts, false)) {
            assert(m_buf->unused() == 0);
            return false;
        }
    }

    //--------------------------------------------------------------------------
    // check if key read is within defined key range (i.e. smaller than endkey)
    //--------------------------------------------------------------------------
    if (m_end_key) {
        if ((cmp = strcmp(tmpkey, m_end_key)) > 0 || (cmp == 0 && m_end_incl == false)) {
            return false;
        }
    }

    *key = tmpkey;
    *value = tmpvalue;
    *keylen = tmpkeylen;
    *valuelen = tmpvaluelen;
    *timestamp = tmpts;
    return true;
}
