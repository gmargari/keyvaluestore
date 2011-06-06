#include "Global.h"
#include "KVDiskFileOutputStream.h"
#include "VFile.h"
#include "KVSerialization.h"

#include <cstdlib>
#include <cassert>

/*========================================================================
 *                           KVDiskFileOutputStream
 *========================================================================*/
KVDiskFileOutputStream::KVDiskFileOutputStream(KVDiskFile *file)
{
    m_kvdiskfile = file;
    m_buf_size = SCANNERBUFSIZE;
    m_buf = (char *)malloc(m_buf_size);
    reset();
}

/*========================================================================
 *                          ~KVDiskFileOutputStream
 *========================================================================*/
KVDiskFileOutputStream::~KVDiskFileOutputStream()
{
    free(m_buf);
}

/*========================================================================
 *                                 reset
 *========================================================================*/
void KVDiskFileOutputStream::reset()
{
    m_bytes_in_buf = 0;
    m_bytes_used = 0;
    m_kvdiskfile->m_vfile->fs_rewind();
}

/*========================================================================
 *                                 write
 *========================================================================*/
bool KVDiskFileOutputStream::write(const char *key, const char *value)
{
    uint32_t len;

    if (serialize(m_buf, m_buf_size, key, value, &len) &&
          m_kvdiskfile->m_vfile->fs_write(m_buf, len)) {
        return true;
    } else {
        return false;
    }
}

/*========================================================================
 *                                flush
 *========================================================================*/
void KVDiskFileOutputStream::flush()
{
    m_kvdiskfile->m_vfile->fs_sync();
}

/*=======================================================================*
 *                             sanity_check
 *=======================================================================*/
void KVDiskFileOutputStream::sanity_check()
{
#if DBGLVL < 2
    return;
#endif
}
