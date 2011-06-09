#include "Global.h"
#include "KVTDiskFileInputStream.h"
#include "VFile.h"
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
    m_buf_size = SCANNERBUFSIZE;
    m_buf = (char *)malloc(m_buf_size);
    reset();
}

/*========================================================================
 *                          ~KVTDiskFileInputStream
 *========================================================================*/
KVTDiskFileInputStream::~KVTDiskFileInputStream()
{
    free(m_buf);
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

    if (deserialize(m_buf + m_bytes_used, m_bytes_in_buf - m_bytes_used, key, value, timestamp, &len, false)) {
        m_bytes_used += len;
        return len;
    } else {

        // keep only unused bytes in buffer
        unused_bytes = m_bytes_in_buf - m_bytes_used;
        memmove(m_buf, m_buf + m_bytes_used, unused_bytes);
        m_bytes_in_buf = unused_bytes;
        m_bytes_used = 0;

        // read more bytes to buffer
        m_bytes_in_buf += m_kvtdiskfile->m_vfile->fs_read(m_buf, m_buf_size - m_bytes_used);
        if (deserialize(m_buf, m_bytes_in_buf, key, value, timestamp, &len, false)) {
            m_bytes_used += len;
            return len;
        }

        *value = NULL;
        *timestamp = 0;
        return 0;
    }
}

/*=======================================================================*
 *                              sanity_check
 *=======================================================================*/
void KVTDiskFileInputStream::sanity_check()
{
    return_if_dbglvl_lt_2();
}
