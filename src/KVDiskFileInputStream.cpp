#include "Global.h"
#include "KVDiskFileInputStream.h"
#include "VFile.h"
#include "KVSerialization.h"

#include <cstdlib>
#include <cstring>
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
    m_kvdiskfile->m_vfile->fs_rewind();
}

/*========================================================================
 *                                 read
 *========================================================================*/
bool KVDiskFileInputStream::read(const char **key, const char **value)
{
    uint32_t len, unused_bytes;

    // NOTE: 'false' arg: do not copy key and value, just make them point to
    // the buffer position, as read from fs_read().
    // NOTE: pointers are valid only until next call to deserialize. if we want
    // to used them after next call to deserialize() we must copy key and value.
    if (deserialize(m_buf + m_bytes_used, m_bytes_in_buf - m_bytes_used, key, value, &len, false)) {
        m_bytes_used += len;
        return len;
    } else {

        // keep only unused bytes
        unused_bytes = m_bytes_in_buf - m_bytes_used;
        memmove(m_buf, m_buf + m_bytes_used, unused_bytes);
        m_bytes_in_buf = unused_bytes;
        m_bytes_used = 0;

        // read more bytes to buffer
        m_bytes_in_buf += m_kvdiskfile->m_vfile->fs_read(m_buf, m_buf_size - m_bytes_used);
        if (deserialize(m_buf, m_bytes_in_buf, key, value, &len, false)) {
            m_bytes_used += len;
            return len;
        }

        return 0;
    }    
}

/*=======================================================================*
 *                              sanity_check
 *=======================================================================*/
void KVDiskFileInputStream::sanity_check()
{
#if DBGLVL < 2
    return;
#endif
}
