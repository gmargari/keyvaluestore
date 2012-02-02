// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#include "./Global.h"
#include "./DiskFileInputStream.h"

#include <string.h>
#include <assert.h>

#include "./DiskFile.h"
#include "./Buffer.h"

/*============================================================================
 *                             DiskFileInputStream
 *============================================================================*/
DiskFileInputStream::DiskFileInputStream(DiskFile *file)
    : m_diskfile(file), m_offs(0), m_buf(NULL), m_start_key(), m_end_key(),
      m_start_incl(true), m_end_incl(true) {
    m_buf = new Buffer(MERGEBUF_SIZE);
    set_key_range(Slice(NULL, 0), Slice(NULL, 0));
}

/*============================================================================
 *                             DiskFileInputStream
 *============================================================================*/
DiskFileInputStream::DiskFileInputStream(DiskFile *file, uint32_t bufsize)
    : m_diskfile(file), m_offs(0), m_buf(NULL), m_start_key(), m_end_key(),
      m_start_incl(true), m_end_incl(true) {
    m_buf = new Buffer(bufsize);
    set_key_range(Slice(NULL, 0), Slice(NULL, 0));
}

/*============================================================================
 *                             DiskFileInputStream
 *============================================================================*/
DiskFileInputStream::DiskFileInputStream(DiskFile *file, char *buf,
                                         uint32_t bufsize)
    : m_diskfile(file), m_offs(0), m_buf(NULL), m_start_key(), m_end_key(),
      m_start_incl(true), m_end_incl(true) {
    m_buf = new Buffer(buf, bufsize);
    set_key_range(Slice(NULL, 0), Slice(NULL, 0));
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
void DiskFileInputStream::set_key_range(Slice start_key, Slice end_key,
                                        bool start_incl, bool end_incl) {
    m_start_key = start_key;
    m_end_key = end_key;
    m_start_incl = start_incl;
    m_end_incl = end_incl;

    m_buf->clear();
}

/*============================================================================
 *                              set_key_range
 *============================================================================*/
void DiskFileInputStream::set_key_range(Slice start_key, Slice end_key) {
    set_key_range(start_key, end_key, true, false);
}

/*============================================================================
 *                                  read
 *============================================================================*/
bool DiskFileInputStream::read(Slice *key, Slice *value, uint64_t *timestamp) {
    int cmp;
    off_t off1, off2;
    Slice tmpkey, tmpvalue;
    uint64_t tmpts;
    bool ret;

    *key = Slice(NULL, 0);
    *value = Slice(NULL, 0);
    *timestamp = 0;

    //--------------------------------------------------------------------------
    // this code is executed only the first time read() is called, after reset()
    // or set_key_range(): seek to the first valid key on disk
    //--------------------------------------------------------------------------
    if (m_buf->size() == 0) {

        assert(m_buf->used() == 0);

        if (m_start_key.data()) {  // seek to first disk key > or >= 'startkey'

            // if m_start_key was stored on disk it would be between off1 & off2
            ret = m_diskfile->search(m_start_key, &off1, &off2);
            if (ret == false) {
                return false;
            }

            // read in buffer all bytes between 'off1' and 'off2'
            m_buf->clear();
            m_diskfile->fill(m_buf, off2 - off1, off1);
            m_offs = off2;
            assert(m_buf->size() == off2 - off1);

            // check all buffer tuples until we find a key >= startkey
            while (m_buf->deserialize(&tmpkey, &tmpvalue, &tmpts, false)) {
                if ((cmp = strcmp(tmpkey.data(), m_start_key.data())) >= 0) {
                    break;
                }
            }

            if (cmp > 0 || (cmp == 0 && m_start_incl == true)) {
                // must seek back at beginning of key tuple
                m_buf->undo_deserialize(tmpkey, tmpvalue, tmpts);
            }
        } else {  // seek to first disk key
            m_offs = 0;
            m_offs += m_diskfile->fill(m_buf, m_offs);
        }
    }

    //--------------------------------------------------------------------------
    // read next <key,value,timestamp> from file
    //--------------------------------------------------------------------------
    if (!m_buf->deserialize(&tmpkey, &tmpvalue, &tmpts, false)) {

        // maybe we need to read more bytes in buffer to deserialize tuple
        m_buf->keep_unused();
        m_offs += m_diskfile->fill(m_buf, m_offs);

        // this should now work, unless there are no bytes left in file
        if (!m_buf->deserialize(&tmpkey, &tmpvalue, &tmpts, false)) {
            assert(m_buf->unused() == 0);
            return false;
        }
    }

    //--------------------------------------------------------------------------
    // check if key read is within defined key range (i.e. smaller than endkey)
    //--------------------------------------------------------------------------
    if (m_end_key.data()) {
        if ((cmp = strcmp(tmpkey.data(), m_end_key.data())) > 0
              || (cmp == 0 && m_end_incl == false)) {
            return false;
        }
    }

    *key = tmpkey;
    *value = tmpvalue;
    *timestamp = tmpts;
    return true;
}
