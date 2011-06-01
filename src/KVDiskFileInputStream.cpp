#include "Global.h"
#include "KVDiskFileInputStream.h"
#include "VFile.h"

#include <cstdlib>
#include <cassert>

/*========================================================================
 *                           KVDiskFileInputStream
 *========================================================================*/
KVDiskFileInputStream::KVDiskFileInputStream(KVDiskFile *file)
{
    m_kvdiskfile = file;
    m_buf_size = SCANNERBUFSIZE;
    m_buf = (char *)malloc(m_buf_size);
    reset();
}

/*========================================================================
 *                          ~KVDiskFileInputStream
 *========================================================================*/
KVDiskFileInputStream::~KVDiskFileInputStream()
{
    free(m_buf);
}

/*========================================================================
 *                                 reset
 *========================================================================*/
void KVDiskFileInputStream::reset()
{
    m_bytes_in_buf = 0;
    m_bytes_used = 0;
    m_kvdiskfile->rewind();
}

/*========================================================================
 *                                 next
 *========================================================================*/
bool KVDiskFileInputStream::read(const char **key, const char **value)
{

    // TODO: static variables should be private for while class. code below should handle
    // case file is closed and reopen, case we performed a read and then a write
    // etc (e.g. we may read a file, then rewind, and read it again..)
    uint32_t len;

    if (m_bytes_in_buf == 0) { // first time function called
        m_bytes_in_buf = m_kvdiskfile->m_vfile->fs_read(m_buf, 200);
        assert(m_bytes_in_buf);
    }

    if (m_bytes_used == m_bytes_in_buf) {
        return 0;
    }

    // NOTE: 'false' arg: do not copy key and value, just make them point to
    // the buffer position, as read from fs_read().
    // NOTE: pointers are valid only until next call to deserialize. if we want
    // to used them after next call to deserialize() we must copy key and value.
    if (deserialize(m_buf + m_bytes_used, m_bytes_in_buf - m_bytes_used, key, value, &len, false)) {
        m_bytes_used += len;
        return len;
    } else {
        // TODO: read more bytes to disk, preserving existing unused bytes
        return 0;
    }
    
}
