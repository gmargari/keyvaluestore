#include "Global.h"
#include "DiskFileOutputStream.h"

#include "DiskFile.h"
#include "VFile.h"
#include "VFileIndex.h"
#include "Buffer.h"

#include <cstdlib>
#include <cassert>

/*============================================================================
 *                            DiskFileOutputStream
 *============================================================================*/
DiskFileOutputStream::DiskFileOutputStream(DiskFile *file, uint32_t bufsize)
    : m_diskfile(file), m_vfile_size(0), m_buf(NULL), m_last_key(NULL),
      m_last_offs(-1), m_last_idx_offs(-1)
{
    m_buf = new Buffer(bufsize);
    m_last_key = (char *)malloc(MAX_KVTSIZE);
    reset();
}

/*============================================================================
 *                           ~DiskFileOutputStream
 *============================================================================*/
DiskFileOutputStream::~DiskFileOutputStream()
{
    delete m_buf;
    free(m_last_key);
}

/*============================================================================
 *                                 reset
 *============================================================================*/
void DiskFileOutputStream::reset()
{
    m_diskfile->m_vfile->fs_rewind();   // contents will be overwritten
    m_diskfile->m_vfile_index->clear(); // index will be rebuild
    m_diskfile->m_vfile_numkeys = 0;

    m_buf->m_bytes_in_buf = 0;

    m_last_key[0] = '\0';
    m_last_offs = -1;
    m_last_idx_offs = -1;
}

/*============================================================================
 *                                  write
 *============================================================================*/
bool DiskFileOutputStream::write(const char *key, size_t keylen, const char *value, size_t valuelen, uint64_t timestamp)
{
    uint32_t len;
    off_t cur_offs;

    // if there is not enough space in buffer for new <k,v> pair, flush buffer
    if (m_buf->m_bytes_in_buf + Buffer::serialize_len(keylen, valuelen, timestamp) > m_buf->m_buf_size) {
        m_diskfile->m_vfile->fs_write(m_buf);
        m_buf->m_bytes_in_buf = 0;
    }

    // serialize and add new pair to buffer
    if (m_buf->serialize(key, keylen, value, valuelen, timestamp, &len)) {

        // if needed, add entry to index
        cur_offs = m_vfile_size;
        if (m_last_idx_offs == -1 || cur_offs + len - m_last_idx_offs > MAX_INDEX_DIST) {
            m_diskfile->m_vfile_index->add(key, cur_offs);
            m_last_idx_offs = cur_offs;
        }

        m_diskfile->m_vfile_numkeys++;
        m_vfile_size += len;
        m_diskfile->m_vfile_index->set_vfilesize(m_vfile_size);

        strcpy(m_last_key, key); // TODO: can we avoid this memcpy?
        m_last_offs = cur_offs;
        return true;
    } else {
        return false;
    }
}

/*============================================================================
 *                                 flush
 *============================================================================*/
void DiskFileOutputStream::flush()
{
    m_diskfile->m_vfile->fs_write(m_buf);
    m_diskfile->m_vfile->fs_sync();

    // TODO: this is possibly wrong, someone could call flush many times and not
    // just once at the end. should we add a function like 'close()' or
    // 'finalize()' and move this code there?!
    if (m_last_idx_offs != m_last_offs) {
        m_diskfile->m_vfile_index->add(m_last_key, m_last_offs);
    }
}
