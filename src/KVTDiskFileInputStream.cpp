#include "Global.h"
#include "KVTDiskFileInputStream.h"
#include "KVTDiskFile.h"
#include "VFile.h"
#include "VFileIndex.h"
#include "KVTSerialization.h"

#include <cstdlib>
#include <cstring>
#include <cassert>

/*========================================================================
 *                           KVTDiskFileInputStream
 *========================================================================*/
KVTDiskFileInputStream::KVTDiskFileInputStream(KVTDiskFile *file)
{
    m_kvtdiskfile = file;
    assert(file->m_vfile_index);
    m_buf_capacity = SCANNERBUFSIZE;
    m_buf = (char *)malloc(m_buf_capacity);
    set_buf_size_min();
    set_key_range(NULL, NULL, true, true);
}

/*========================================================================
 *                          ~KVTDiskFileInputStream
 *========================================================================*/
KVTDiskFileInputStream::~KVTDiskFileInputStream()
{
    free(m_buf);
}

/*========================================================================
 *                             set_key_range
 *========================================================================*/
void KVTDiskFileInputStream::set_key_range(const char *start_key, const char *end_key, bool start_incl, bool end_incl)
{
    off_t off1, off2;
    const char *key, *value;
    uint64_t timestamp;
    uint32_t len;
    int cmp;
    bool ret;

    m_start_key = start_key;
    m_end_key = end_key;
    m_start_incl = start_incl;
    m_end_incl = end_incl;
    reset();

    if (m_start_key) {

        // if 'start_key' was stored on disk, it would be stored between 'off1' & 'off2'
        ret = m_kvtdiskfile->m_vfile_index->search(m_start_key, &off1, &off2);
        if (ret == false) {

            // 'start_key' was either (lexicographically) smaller than all terms,
            // or greater than all terms in file. lseek() to the end of file
            // so next read() will return false
            m_kvtdiskfile->m_vfile->fs_seek(0, SEEK_END);
            return;
        }

        m_kvtdiskfile->m_vfile->fs_seek(off1, SEEK_SET);

        // check all tuples in buffer, until we find 'start_key' or the next
        // greater term.
        m_bytes_used = 0;
        m_bytes_in_buf = m_kvtdiskfile->m_vfile->fs_read(m_buf, m_buf_size);
        while (deserialize(m_buf + m_bytes_used, m_bytes_in_buf - m_bytes_used, &key, &value, &timestamp, &len, false)) {

            m_bytes_used += len;

            // found 'start_key'
            if ((cmp = strcmp(key, start_key)) == 0) {
                if (m_start_incl == true) {

                    // must seek file at the beginning of 'start_key' tuple
                    assert(m_bytes_used >= serialize_len(strlen(key), strlen(value), timestamp));
                    m_bytes_used -= serialize_len(strlen(key), strlen(value), timestamp);
                    break;
                } else {
                    break; // ok, we're at first term greater then 'start_key'
                }
            } else if (cmp > 0) {
                break; // ok, we're at first term greater then 'start_key'
            }
        }

        assert(m_kvtdiskfile->m_vfile->fs_tell() >= off1);
        assert(m_kvtdiskfile->m_vfile->fs_tell() <= off1 + MAX_INDEX_DIST);

    } else {
        m_kvtdiskfile->m_vfile->fs_rewind();
    }
}

/*========================================================================
 *                             set_key_range
 *========================================================================*/
void KVTDiskFileInputStream::set_key_range(const char *start_key, const char *end_key)
{
    set_key_range(start_key, end_key, true, false);
}

/*========================================================================
 *                                 reset
 *========================================================================*/
void KVTDiskFileInputStream::reset()
{
    m_bytes_in_buf = 0;
    m_bytes_used = 0;
    m_kvtdiskfile->m_vfile->fs_rewind();
}

/*========================================================================
 *                                 read
 *========================================================================*/
bool KVTDiskFileInputStream::read(const char **key, const char **value, uint64_t *timestamp)
{
    uint32_t len, unused_bytes;
    int cmp;

    // TODO: code repetition, how could I shorten code?

    if (deserialize(m_buf + m_bytes_used, m_bytes_in_buf - m_bytes_used, key, value, timestamp, &len, false)) {

        // check if we reached 'end_key'
        if (m_end_key) {
            if ((cmp = strcmp(*key, m_end_key)) > 0
                  || (cmp == 0 && m_end_incl == false)) {
                return false;
            }
        }

        m_bytes_used += len;
        return true;
    }

    /*
     * maybe we need to read more bytes in buffer to deserialize tuple
     */

    // keep only unused bytes in buffer
    unused_bytes = m_bytes_in_buf - m_bytes_used;
    memmove(m_buf, m_buf + m_bytes_used, unused_bytes);
    m_bytes_in_buf = unused_bytes;
    m_bytes_used = 0;
    // read more bytes to buffer
    m_bytes_in_buf += m_kvtdiskfile->m_vfile->fs_read(m_buf + m_bytes_in_buf, m_buf_size - m_bytes_in_buf);

    if (deserialize(m_buf, m_bytes_in_buf, key, value, timestamp, &len, false)) {

        // check if we reached 'end_key'
        if (m_end_key) {
            if ((cmp = strcmp(*key, m_end_key)) > 0
                || (cmp == 0 && m_end_incl == false)) {
                return false;
            }
        }

        m_bytes_used += len;
        return true;
    }

    // no more bytes left in file
    assert(m_bytes_in_buf - m_bytes_used == 0);
    *key = NULL;
    *value = NULL;
    *timestamp = 0;

    return false;
}

/*=======================================================================*
 *                            set_buf_size_min
 *=======================================================================*/
void KVTDiskFileInputStream::set_buf_size_min()
{
    m_buf_size = MAX_INDEX_DIST;
}

/*=======================================================================*
 *                            set_buf_size_max
 *=======================================================================*/
void KVTDiskFileInputStream::set_buf_size_max()
{
    m_buf_size = m_buf_capacity;
}

/*=======================================================================*
 *                             set_buf_size
 *=======================================================================*/
void KVTDiskFileInputStream::set_buf_size(uint32_t size)
{
    if (size <= m_buf_capacity) {
        m_buf_size = size;
    } else {
        assert("size > m_buf_capacity" && 0);
    }
}

/*=======================================================================*
 *                             get_buf_size
 *=======================================================================*/
uint32_t KVTDiskFileInputStream::get_buf_size()
{
    return m_buf_size;
}

/*=======================================================================*
 *                           get_buf_capacity
 *=======================================================================*/
uint32_t KVTDiskFileInputStream::get_buf_capacity()
{
    return m_buf_capacity;
}

/*=======================================================================*
 *                              sanity_check
 *=======================================================================*/
void KVTDiskFileInputStream::sanity_check()
{
    return_if_dbglvl_lt_2();
}
