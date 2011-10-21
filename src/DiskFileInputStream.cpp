#include "Global.h"
#include "DiskFileInputStream.h"
#include "DiskFile.h"
#include "VFile.h"
#include "VFileIndex.h"
#include "Buffer.h"

#include <cstdlib>
#include <cstring>
#include <cassert>

/*============================================================================
 *                             DiskFileInputStream
 *============================================================================*/
DiskFileInputStream::DiskFileInputStream(DiskFile *file, uint32_t bufsize)
    : m_diskfile(file), m_buf(NULL), m_start_key(NULL), m_end_key(NULL), m_start_incl(true),
      m_end_incl(true)
{
    m_buf = new Buffer(bufsize);
    assert(file->m_vfile_index);
}

/*============================================================================
 *                            ~DiskFileInputStream
 *============================================================================*/
DiskFileInputStream::~DiskFileInputStream()
{
    delete m_buf;
}

/*============================================================================
 *                                set_key_range
 *============================================================================*/
void DiskFileInputStream::set_key_range(const char *start_key, const char *end_key, bool start_incl, bool end_incl)
{
    m_start_key = start_key;
    m_end_key = end_key;
    m_start_incl = start_incl;
    m_end_incl = end_incl;

    m_buf->clear();
}

/*============================================================================
 *                              set_key_range
 *============================================================================*/
void DiskFileInputStream::set_key_range(const char *start_key, const char *end_key)
{
    set_key_range(start_key, end_key, true, false);
}

/*============================================================================
 *                                  read
 *============================================================================*/
bool DiskFileInputStream::read(const char **key, const char **value, uint64_t *timestamp)
{
    uint32_t len;
    int cmp;
    off_t off1, off2;
    const char *tmpkey, *tmpvalue;
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
            ret = m_diskfile->m_vfile_index->search(m_start_key, &off1, &off2);
            if (ret == false) {
                return false;
            }

            // read in buffer all bytes between 'off1' and 'off2'
            m_buf->clear();
            m_diskfile->m_vfile->fs_seek(off1, SEEK_SET);
            m_buf->fill(m_diskfile->m_vfile, off2 - off1);

            // check all buffer tuples until we find a key >= startkey
            while (m_buf->deserialize(&tmpkey, &tmpvalue, &tmpts, &len, false)) {
                if ((cmp = strcmp(tmpkey, m_start_key)) >= 0) {
                    break;
                }
            }

            if (cmp > 0 || (cmp == 0 && m_start_incl == true)) {
                // must seek back at beginning of key tuple
                m_buf->m_bytes_used -= len;
            }

            assert(m_diskfile->m_vfile->fs_tell() >= off1);
            assert(m_diskfile->m_vfile->fs_tell() <= off1 + MAX_INDEX_DIST);
        }
        // else, seek to the first key on disk
        else {
            m_diskfile->m_vfile->fs_rewind();
            m_buf->fill(m_diskfile->m_vfile);
        }
    }

    //--------------------------------------------------------------------------
    // read next <key,value,timestamp> from file
    //--------------------------------------------------------------------------
    if ( ! m_buf->deserialize(&tmpkey, &tmpvalue, &tmpts, &len, false)) {

        // maybe we need to read more bytes in buffer to deserialize tuple
        m_buf->keep_unused();
        m_buf->fill(m_diskfile->m_vfile);

        // this should now work, unless there are no bytes left in file
        if ( ! m_buf->deserialize(&tmpkey, &tmpvalue, &tmpts, &len, false)) {
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
    *timestamp = tmpts;
    return true;
}
