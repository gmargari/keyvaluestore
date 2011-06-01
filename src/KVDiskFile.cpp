#include "Global.h"
#include "KVDiskFile.h"
#include "KVSerialization.h"

#include <cassert>

/*========================================================================
 *                               KVDiskFile
 *========================================================================*/
KVDiskFile::KVDiskFile()
{
    m_vfile = new VFile(true);


    // TODO: tmp hack!
    m_bytes_in_buf = 0;
    m_bytes_used = 0;
}

/*========================================================================
 *                              ~KVDiskFile
 *========================================================================*/
KVDiskFile::~KVDiskFile()
{
    delete m_vfile;
}

#include <stdio.h>

/*========================================================================
 *                                 write
 *========================================================================*/
void KVDiskFile::write(const char *key, const char *value)
{
    uint32_t len;

    serialize(m_buf, MBFUSIZE, key, value, &len);
    m_vfile->fs_write(m_buf, len);
}

// /*========================================================================
//  *                                 read
//  *========================================================================*/
// uint32_t KVDiskFile::read(const char **key, const char **value)
// {
//     // TODO: static variables should be private for while class. code below should handle
//     // case file is closed and reopen, case we performed a read and then a write
//     // etc (e.g. we may read a file, then rewind, and read it again..)
//     uint32_t len;
//     
//     if (m_bytes_in_buf == 0) { // first time function called
//         m_bytes_in_buf = m_vfile->fs_read(m_buf, 200);
//         assert(m_bytes_in_buf);
//     }
// 
//     if (m_bytes_used == m_bytes_in_buf) {
//         return 0;
//     }
// 
//     // NOTE: 'false' arg: do not copy key and value, just make them point to 
//     // the buffer position, as read from fs_read(). 
//     // NOTE: pointers are valid only until next call to deserialize. if we want
//     // to used them after next call to deserialize() we must copy key and value.
//     if (deserialize(m_buf + m_bytes_used, 1000, key, value, &len, false)) { 
//         m_bytes_used += len;
//         return len;
//     } else {
//         // TODO: read more bytes to disk, preserving existing unused bytes
//         return 0;
//     }
// }

/*========================================================================
 *                                rewind
 *========================================================================*/
void KVDiskFile::rewind()
{
    m_vfile->fs_rewind();
}

/*========================================================================
 *                                 flush
 *========================================================================*/
void KVDiskFile::sync()
{
    m_vfile->fs_sync();
}

/*========================================================================
 *                                 close
 *========================================================================*/
void KVDiskFile::close()
{
    m_vfile->fs_close();
}

/*========================================================================
 *                                 close
 *========================================================================*/
char *KVDiskFile::name()
{
    return m_vfile->fs_name();
}