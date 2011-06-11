#include "Global.h"
#include "KVTDiskFileOutputStream.h"
#include "VFile.h"
#include "KVTSerialization.h"

#include <cstdlib>
#include <cassert>

/*========================================================================
 *                           KVTDiskFileOutputStream
 *========================================================================*/
KVTDiskFileOutputStream::KVTDiskFileOutputStream(KVTDiskFile *file)
{
    m_vfile = file->m_vfile;
    m_vfile_index = &(file->m_vfile_index);
    m_vfile_numkeys = &(file->m_vfile_numkeys);

    m_buf_size = SCANNERBUFSIZE;
    m_buf = (char *)malloc(m_buf_size);
    reset();
}

/*========================================================================
 *                          ~KVTDiskFileOutputStream
 *========================================================================*/
KVTDiskFileOutputStream::~KVTDiskFileOutputStream()
{
    free(m_buf);
}

/*========================================================================
 *                                 reset
 *========================================================================*/
void KVTDiskFileOutputStream::reset()
{
    m_vfile->fs_rewind();   // contents will be overwritten
    m_vfile_index->clear(); // index will be rebuild
    (*m_vfile_numkeys) = 0;

    m_bytes_in_buf = 0;
    m_bytes_used = 0;
    m_lastoffs = -1;
    m_vfile_size = 0;
}

/*========================================================================
 *                                 write
 *========================================================================*/
bool KVTDiskFileOutputStream::write(const char *key, const char *value, uint64_t timestamp)
{
    uint32_t len;
    off_t cur_offs;

    if (serialize(m_buf, m_buf_size, key, value, timestamp, &len) &&
          m_vfile->fs_write(m_buf, len)) {

        // if needed, add entry to index
        cur_offs = m_vfile_size;
        if (m_lastoffs == -1 || cur_offs + len - m_lastoffs > MAX_INDEX_DIST) {
            m_vfile_index->add(key, cur_offs);
            m_lastoffs = cur_offs;
        }

        (*m_vfile_numkeys)++;
        m_vfile_size += len;
        assert(m_vfile->fs_size() == m_vfile_size);
        m_vfile_index->set_vfilesize(m_vfile_size);
        return true;
    } else {
        return false;
    }
}

/*========================================================================
 *                                flush
 *========================================================================*/
void KVTDiskFileOutputStream::flush()
{
    m_vfile->fs_sync();
}

/*=======================================================================*
 *                             sanity_check
 *=======================================================================*/
void KVTDiskFileOutputStream::sanity_check()
{
    return_if_dbglvl_lt_2();
}
