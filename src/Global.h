// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_GLOBAL_H_
#define SRC_GLOBAL_H_

#include <stdlib.h>
#include <stdint.h>

// set debug level:
//  0 - debugging disabled
//  1 - normal mode     (assertions and sanity_check()s enabled)
//  2 - heavy debugging (assertions, sanity_checks()s and all other
//                       debugging functions enabled)
#define DBGLVL 0

//==============================================================================
// default values
//==============================================================================
// size of memstore
const uint64_t DEFAULT_MEMSTORE_SIZE = 104857600;

// max size of each disk file created (limit probably imposed by file system)
const uint64_t MAX_FILE_SIZE = 2147483647;

// max size for key or value
const uint32_t MAX_KEY_SIZE = 1024;
const uint32_t MAX_VALUE_SIZE = 102400;

// I/O operations from/to disk for compactions are done in units of this size.
// 'MERGEBUF_SIZE' must be at least:
//   2 * sizeof(uint64_t) + MAX_KEY_SIZE + MAX_VALUE_SIZE + sizeof(uint64_t)
// (e.g. to be able read from disk a KV: <klen, vlen, key, value, timestamp>)
const uint64_t MERGEBUF_SIZE = 524288;

// chunk size: keep in memory the key that appears every 'CHUNK_SIZE' bytes on
// disk files, along with the exact offset it appears.
// reads from disk for get() operations are done in units of chunks.
// decreasing this means we read less bytes for each get(), but increases the
// size of the in-memory chunk index (BigTable, HBase and Hypertable use 64KB)
const uint32_t CHUNK_SIZE = 65536;

//==============================================================================
// compaction managers' default values
//==============================================================================
const int      DEFAULT_GEOM_R = 3;
const int      DEFAULT_GEOM_P = 0;
const uint64_t DEFAULT_RNG_BLOCKSIZE = 268435456;
const int      DEFAULT_CASS_K = 4;

//==============================================================================
// vfile-related defines
//==============================================================================
#define ROOT_DIR            "/tmp/kvstore/"  // dont forget trailing '/'
#define CMMANAGER_FILENAME  "cmmanager.info"
#define DISKSTORE_FILENAME  "dstore.info"
#define DISKFILE_PREFIX     "dfile."
#define VFILE_PART_PREFIX   ".part"
#define VFILE_INFO_SUFFIX   ".info"
#define VFILE_INDEX_SUFFIX  ".index"

//==============================================================================
// macros to convert bytes
//==============================================================================

#define b2kb(b) ((b)/(1024.0))
#define b2mb(b) ((b)/(1024.0*1024.0))
#define b2gb(b) ((b)/(1024.0*1024.0*1024.0))

#define kb2b(kb) ((uint64_t)((kb)*(1024)))
#define mb2b(mb) ((uint64_t)((mb)*(1024*1024)))
#define gb2b(gb) ((uint64_t)((gb)*(1024*1024*1024)))

#define kb2mb(kb) ((kb)/(1024.0))
#define kb2gb(kb) ((kb)/(1024.0*1024.0))

#define mb2kb(mb) ((uint64_t)((mb)*(1024)))
#define gb2kb(gb) ((uint64_t)((gb)*(1024*1024)))

#define mb2gb(mb) ((mb)/(1024.0))

#define gb2mb(gb) ((uint64_t)((gb)*(1024)))

// byte conversion
#define sec2hour(s) ((s)/3600.0)
#define sec2min(s)  ((s)/60.0)
#define sec2msec(s) ((s)*1000)
#define sec2usec(s) ((suseconds_t)((s)*1000000))

#define hour2sec(h) ((h)*3600.0)
#define min2sec(m)  ((m)*60.0)
#define msec2sec(m) ((m)/1000.0)
#define usec2sec(u) ((u)/1000000.0)

#define usec2msec(u) ((u)/1000.0)

#include "./Debug.h"

#endif  // SRC_GLOBAL_H_
