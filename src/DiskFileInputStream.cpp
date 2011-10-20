#include "Global.h"
#include "DiskFileInputStream.h"
#include "DiskFile.h"
#include "VFile.h"
#include "VFileIndex.h"
#include "Serialization.h"
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

    m_buf->m_bytes_in_buf = 0;
    m_buf->m_bytes_used = 0;
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
    uint32_t len, unused_bytes;
    int cmp;
    off_t off1, off2;
    const char *disk_key, *disk_value;
    bool ret;

    //--------------------------------------------------------------------------
    // this code is executed only the first time read() is called, after reset()
    // or set_key_range(): seek to the first valid key on disk
    //--------------------------------------------------------------------------
    if (m_buf->m_bytes_in_buf == 0) {

        // if caller defined a start_key, seek to the first valid key on disk
        if (m_start_key) {

            // if 'm_start_key' was stored on disk, it would be stored between 'off1' & 'off2'
            ret = m_diskfile->m_vfile_index->search(m_start_key, &off1, &off2);
            if (ret == false) {
                // 'm_start_key' was either (lexicographically) smaller than all terms,
                // or greater than all terms in file.
                return false;
            }

            // read in buffer all bytes between 'off1' and 'off2'. check all tuples
            // in buffer until we find 'm_start_key' or the next greater term.
            m_diskfile->m_vfile->fs_seek(off1, SEEK_SET);
            m_buf->m_bytes_in_buf = m_diskfile->m_vfile->fs_read(m_buf, off2 - off1);
            m_buf->m_bytes_used = 0;
            while (deserialize(m_buf, &disk_key, &disk_value, timestamp, &len, false)) {

                // found 'm_start_key'
                if ((cmp = strcmp(disk_key, m_start_key)) == 0) {
                    if (m_start_incl == true) {
                        // must seek file back at the beginning of 'm_start_key' tuple
                        m_buf->m_bytes_used -= len;
                    }
                    break;
                }
                // first term greater than 'm_start_key'
                else if (cmp > 0) {
                    break;
                }
            }

            // in special case where 'm_start_key' == 'm_end_key' (for example,
            // DiskStore::get()) return false as term does not exist on disk
            if (m_buf->m_bytes_used == m_buf->m_bytes_in_buf && m_end_key && strcmp(m_start_key, m_end_key) == 0) {
                return false;
            }

            assert(m_diskfile->m_vfile->fs_tell() >= off1);
            assert(m_diskfile->m_vfile->fs_tell() <= off1 + MAX_INDEX_DIST);
        }
        // else, seek to the first key on disk
        else {
            m_diskfile->m_vfile->fs_rewind();
        }
    }

    // TODO: code repetition, how could I shorten code?
    if (deserialize(m_buf, key, value, timestamp, &len, false)) {

        // check if we reached 'end_key'
        if (m_end_key && ((cmp = strcmp(*key, m_end_key)) > 0 || (cmp == 0 && m_end_incl == false))) {
            return false;
        }

        return true;
    }

    /*
     * maybe we need to read more bytes in buffer to deserialize tuple
     */

    // keep only unused bytes in buffer
    unused_bytes = m_buf->m_bytes_in_buf - m_buf->m_bytes_used;
    memmove(m_buf->m_buf, m_buf->m_buf + m_buf->m_bytes_used, unused_bytes);
    m_buf->m_bytes_in_buf = unused_bytes;
    m_buf->m_bytes_used = 0;
    // read more bytes to buffer
    m_buf->m_bytes_in_buf += m_diskfile->m_vfile->fs_read(m_buf, m_buf->m_buf_size - m_buf->m_bytes_in_buf);

    if (deserialize(m_buf, key, value, timestamp, &len, false)) {

        // check if we reached 'end_key'
        if (m_end_key && ((cmp = strcmp(*key, m_end_key)) > 0 || (cmp == 0 && m_end_incl == false))) {
            return false;
        }

        return true;
    }

    // no more bytes left in file
    assert(m_buf->m_bytes_in_buf - m_buf->m_bytes_used == 0);
    *key = NULL;
    *value = NULL;
    *timestamp = 0;

    return false;
}
