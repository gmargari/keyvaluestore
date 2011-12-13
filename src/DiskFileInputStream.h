// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_DISKFILEINPUTSTREAM_H_
#define SRC_DISKFILEINPUTSTREAM_H_

#include <sys/types.h>

#include "./Global.h"
#include "./InputStream.h"
#include "./Slice.h"

class DiskFile;
class Buffer;

class DiskFileInputStream: public InputStream {
  public:
    /**
     * constructor
     */
    DiskFileInputStream(DiskFile *file, uint32_t bufsize);

    /**
     * constructor
     */
    DiskFileInputStream(DiskFile *file, char *buf, uint32_t bufsize);

    /**
     * destructor
     */
    ~DiskFileInputStream();

    // inherited from InputStream
    void set_key_range(Slice start_key, Slice end_key, bool start_incl, bool end_incl);
    void set_key_range(Slice start_key, Slice end_key);
    bool read(Slice *key, Slice *value, uint64_t *timestamp);

    // Undefined methods (just remove Weffc++ warning)
    DiskFileInputStream(const DiskFileInputStream&);
    DiskFileInputStream& operator=(const DiskFileInputStream&);

  private:
    DiskFile *m_diskfile;
    off_t     m_offs;        // offset within diskfile we currently are
    Buffer   *m_buf;         // buffer used for I/O
    Slice     m_start_key;
    Slice     m_end_key;
    bool      m_start_incl;
    bool      m_end_incl;
};

#endif  // SRC_DISKFILEINPUTSTREAM_H_
