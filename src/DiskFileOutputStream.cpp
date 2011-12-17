// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#include "./Global.h"
#include "./DiskFileOutputStream.h"

#include <stdlib.h>

#include "./DiskFile.h"
#include "./VFileIndex.h"
#include "./Buffer.h"

/*============================================================================
 *                            DiskFileOutputStream
 *============================================================================*/
DiskFileOutputStream::DiskFileOutputStream(DiskFile *file)
    : m_file(file), m_file_size(0), m_buf(NULL), m_index(NULL),
      m_last_key(NULL), m_last_keylen(0), m_last_offs(-1), m_last_idx_offs(-1),
      m_stored_keys(0) {
    m_buf = new Buffer(MERGE_BUFSIZE);
    m_last_key = (char *)malloc(MAX_KVTSIZE);
    m_index = new VFileIndex();
}

/*============================================================================
 *                            DiskFileOutputStream
 *============================================================================*/
DiskFileOutputStream::DiskFileOutputStream(DiskFile *file, uint32_t bufsize)
    : m_file(file), m_file_size(0), m_buf(NULL), m_index(NULL),
      m_last_key(NULL), m_last_keylen(0), m_last_offs(-1), m_last_idx_offs(-1),
      m_stored_keys(0) {
    m_buf = new Buffer(bufsize);
    m_last_key = (char *)malloc(MAX_KVTSIZE);
    m_index = new VFileIndex();
}

/*============================================================================
 *                           ~DiskFileOutputStream
 *============================================================================*/
DiskFileOutputStream::~DiskFileOutputStream() {
    delete m_buf;
    free(m_last_key);
}

/*============================================================================
 *                                 append
 *============================================================================*/
bool DiskFileOutputStream::append(Slice key, Slice value, uint64_t timestamp) {
    uint32_t len;
    off_t cur_offs;

    // if there is not enough space in buffer for new <k,v> pair, flush buffer
    if (Buffer::serialize_len(key.size(), value.size(), timestamp)
          > m_buf->free_space()) {
        m_file->append(m_buf);
    }

    // serialize and add new pair to buffer
    if (m_buf->serialize(key, value, timestamp, &len)) {

        // if needed, add entry to index
        cur_offs = m_file_size;
        if (m_last_idx_offs == -1 ||
              cur_offs + len - m_last_idx_offs > MAX_INDEX_DIST) {
            m_index->add(key, cur_offs);
            m_last_idx_offs = cur_offs;
        }

        m_stored_keys++;
        m_file_size += len;
        m_index->set_vfilesize(m_file_size);

        m_last_keylen = key.size();
        memcpy(m_last_key, key.data(), key.size() + 1);  // TODO: avoid this?
        m_last_offs = cur_offs;
        return true;
    } else {
        return false;
    }
}

/*============================================================================
 *                                 close
 *============================================================================*/
void DiskFileOutputStream::close() {
    m_file->append(m_buf);
    m_file->sync();

    if (m_last_idx_offs != m_last_offs) {
        m_index->add(Slice(m_last_key, m_last_keylen), m_last_offs);
    }
    m_file->set_file_index(m_index);
    m_file->set_num_keys(m_stored_keys);
}
