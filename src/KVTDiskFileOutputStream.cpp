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
    m_kvtdiskfile = file;
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
    m_bytes_in_buf = 0;
    m_bytes_used = 0;
    m_kvtdiskfile->m_vfile->fs_rewind();
}

/*========================================================================
 *                                 write
 *========================================================================*/
bool KVTDiskFileOutputStream::write(const char *key, const char *value, uint64_t timestamp)
{
    uint32_t len;

    if (serialize(m_buf, m_buf_size, key, value, timestamp, &len) &&
          m_kvtdiskfile->m_vfile->fs_write(m_buf, len)) {
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
    m_kvtdiskfile->m_vfile->fs_sync();
}

/*=======================================================================*
 *                             sanity_check
 *=======================================================================*/
void KVTDiskFileOutputStream::sanity_check()
{
    return_if_dbglvl_lt_2();
}
