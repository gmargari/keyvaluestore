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
    : m_diskfile(file), m_offs(0), m_vfile_size(0), m_buf(NULL), m_vfile_index(NULL),
      m_last_key(NULL), m_last_offs(-1), m_last_idx_offs(-1), m_vfile_numkeys(0)
{
    m_buf = new Buffer(bufsize);
    m_last_key = (char *)malloc(MAX_KVTSIZE);
    m_vfile_index = new VFileIndex();
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
    m_offs = 0;
    m_vfile_index->clear(); // index will be rebuild
    m_vfile_numkeys = 0;

    m_buf->clear();

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
    if (Buffer::serialize_len(keylen, valuelen, timestamp) > m_buf->free_space()) {
        m_offs += m_diskfile->flush(m_buf, m_offs);
    }

    // serialize and add new pair to buffer
    if (m_buf->serialize(key, keylen, value, valuelen, timestamp, &len)) {

        // if needed, add entry to index
        cur_offs = m_vfile_size;
        if (m_last_idx_offs == -1 || cur_offs + len - m_last_idx_offs > MAX_INDEX_DIST) {
            m_vfile_index->add(key, cur_offs);
            m_last_idx_offs = cur_offs;
        }

        m_vfile_numkeys++;
        m_vfile_size += len;
        m_vfile_index->set_vfilesize(m_vfile_size);

        strcpy(m_last_key, key); // TODO: can we avoid this memcpy?
        m_last_offs = cur_offs;
        return true;
    } else {
        return false;
    }
}

/*============================================================================
 *                                 close
 *============================================================================*/
void DiskFileOutputStream::close()
{
    m_offs += m_diskfile->flush(m_buf, m_offs);
    m_diskfile->sync();

    if (m_last_idx_offs != m_last_offs) {
        m_vfile_index->add(m_last_key, m_last_offs);
    }
    m_diskfile->set_file_index(m_vfile_index);
    m_diskfile->set_num_keys(m_vfile_numkeys);
}