#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdint.h>

// set debug level:
//  0 - debugging disabled
//  1 - light mode      (only assertions enabled)
//  2 - normal mode     (assertions and sanity_check()s enabled)
//  3 - heavy debugging (assertions, sanity_checks()s and all other debugging functions enabled)
#define DBGLVL 2

// default values
#define DEFAULT_MEMSTORE_SIZE      1073741824LL // 1GB
#define DEFAULT_STATS_ENABLED             false
#define DEFAULT_STATS_STEP         1073741824LL // 1GB
#define DEFAULT_GEOM_R                        3
#define DEFAULT_GEOM_P                        0 // default disabled (no upper bound on number of partitions)
#define DEFAULT_GEOM_P_IS_CONSTANT        false
#define DEFAULT_URF_BLOCKSIZE       268435456LL // 256MB
#define DEFAULT_URF_FLUSHMEMSIZE            0LL  // 0 -> flush biggest range only
#define DEFAULT_INSERTBYTES      104857600000LL // 100GB

#define TMPFILEDIR                         "/tmp/"
#define TMPFILEPREFIX                      "fsim." // 5 chars max
#define MAX_FILE_SIZE              2147483647LL // 2GB

#define READBUFSIZE                ((size_t)(64*1024*1024))
#define MERGEBUFSIZE               ((size_t)(64*1024*1024))
#define MAX_KVSIZE           ((unsigned long)(1*1024*1024)) // 1MB
#define SCANNERBUFSIZE       ((unsigned long)(4*1024*1024)) // 4MB (must be at least 2*MAX_KVSIZE + 2*sizeof(uint64_t))

// byte conversion
#define b2kb(b) ((b)/(1024.0))
#define b2mb(b) ((b)/(1024.0*1024.0))
#define b2gb(b) ((b)/(1024.0*1024.0*1024.0))

#define kb2b(kb) ((numbytes)((kb)*(1024)))
#define mb2b(mb) ((numbytes)((mb)*(1024*1024)))
#define gb2b(gb) ((numbytes)((gb)*(1024*1024*1024)))

#define kb2mb(kb) ((kb)/(1024.0))
#define kb2gb(kb) ((kb)/(1024.0*1024.0))

#define mb2kb(mb) ((numbytes)((mb)*(1024)))
#define gb2kb(gb) ((numbytes)((gb)*(1024*1024)))

#define mb2gb(mb) ((mb)/(1024.0))

#define gb2mb(gb) ((numbytes)((gb)*(1024)))

// byte conversion
#define sec2hour(s) ((s)/3600.0)
#define sec2min(s)  ((s)/60.0)
#define sec2msec(s) ((s)*1000)
#define sec2usec(s) ((suseconds_t)((s)*1000000))

#define hour2sec(h) ((h)*3600.0)
#define min2sec(m)  ((m)*60.0)
#define msec2sec(m) ((m)/1000.0)
#define usec2sec(u) ((u)/1000000.0))

#if DBGLVL == 0
    #define NDEBUG // disable assertions
#endif

#endif
