// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_GLOBAL_H_
#define SRC_GLOBAL_H_

// set debug level:
//  0 - debugging disabled
//  1 - normal mode     (assertions and sanity_check()s enabled)
//  2 - heavy debugging (assertions, sanity_checks()s and all other debugging functions enabled)
#define DBGLVL 0

//==============================================================================
// default values
//==============================================================================
#define DEFAULT_MEMSTORE_SIZE                   104857600LL  // 100MB
#define MAX_INDEX_DIST                                65536  // 64KB
#define MAX_KVTSIZE          ((unsigned long)(1*1024*1024))  // 1MB (max size for key and for value)
#define MERGE_BUFSIZE        ((unsigned long)(4*1024*1024))  // 4MB (must be at least 2*MAX_KVTSIZE + 2*sizeof(uint64_t))

//==============================================================================
// compaction managers' default values
//==============================================================================
#define DEFAULT_GEOM_R                                    3
#define DEFAULT_GEOM_P                                    0  // default disabled (no upper bound on number of partitions)
#define DEFAULT_RNGMERGE_BLOCKSIZE              268435456LL  // 256MB
#define DEFAULT_RNGMERGE_FLUSHMEMSIZE                   0LL  // 0 -> flush biggest range only
#define DEFAULT_CASS_K                                    4

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
#define MAX_FILE_SIZE       2147483647LL    // 2GB

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
