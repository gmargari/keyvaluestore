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

#define STMT( stuff )     do { stuff } while(0)

#define return_if_dbglvl_lt_1() STMT( if (DBGLVL < 1) return; )
#define return_if_dbglvl_lt_2() STMT( if (DBGLVL < 2) return; )
#define return_if_dbglvl_lt_3() STMT( if (DBGLVL < 3) return; )

/* functions to print an expression and its value */
#define DEBUG_PROMPT "[DEBUG] "

/* integers */
#define dbg_i(expr)        STMT( if (DBGLVL) { printf("%s " #expr " = %d\n", DEBUG_PROMPT, expr); fflush(stdout); } )
#define dbg_i2(i1, i2)     STMT( if (DBGLVL) { printf("%s " #i1 " = %d , " #i2 " = %d\n", DEBUG_PROMPT, i1, i2); fflush(stdout); } )

/* long integers */
#define dbg_li(expr)       STMT( if (DBGLVL) { printf("%s " #expr " = %ld\n", DEBUG_PROMPT, expr); fflush(stdout); } )
#define dbg_li2(i1, i2)    STMT( if (DBGLVL) { printf("%s " #i1 " = %ld , " #i2 " = %ld\n", DEBUG_PROMPT, i1, i2); fflush(stdout); } )
#define dbg_l(expr)        dbg_li(expr)
#define dbg_l2(i1, i2)     dbg_li2( i1, i2)

/* unsigned integers */
#define dbg_ui(expr)       STMT( if (DBGLVL) { printf("%s " #expr " = %u\n", DEBUG_PROMPT, expr); fflush(stdout); } )
#define dbg_ui2(i1, i2)    STMT( if (DBGLVL) { printf("%s " #i1 " = %u , " #i2 " = %u\n", DEBUG_PROMPT, i1, i2); fflush(stdout); } )
#define dbg_u(expr)        dbg_ui(expr)
#define dbg_u2(i1, i2)     dbg_ui2( i1, i2)

/* long unsigned integers */
#define dbg_lui(expr)      STMT( if (DBGLVL) { printf("%s " #expr " = %lu\n", DEBUG_PROMPT, expr); fflush(stdout); } )
#define dbg_lui2(i1, i2)   STMT( if (DBGLVL) { printf("%s " #i1 " = %lu , " #i2 " = %lu\n", DEBUG_PROMPT, i1, i2); fflush(stdout); } )
#define dbg_uli(expr)      dbg_lui(expr)
#define dbg_ul(expr)       dbg_lui(expr)
#define dbg_uli2(i1, i2)   dbg_lui2(i1, i2)

/* long long unsigned integers */
#define dbg_lli(expr)      STMT( if (DBGLVL) { printf("%s " #expr " = %Ld\n", DEBUG_PROMPT, expr); fflush(stdout); } )
#define dbg_lli2(i1, i2)   STMT( if (DBGLVL) { printf("%s " #i1 " = %Ld , " #i2 " = %Ld\n", DEBUG_PROMPT, i1, i2); fflush(stdout); } )

/* long long unsigned integers */
#define dbg_llui(expr)     STMT( if (DBGLVL) { printf("%s " #expr " = %Lu\n", DEBUG_PROMPT, expr); fflush(stdout); } )
#define dbg_llui2(i1, i2)  STMT( if (DBGLVL) { printf("%s " #i1 " = %Lu , " #i2 " = %Lu\n", DEBUG_PROMPT, i1, i2); fflush(stdout); } )
#define dbg_ulli(expr)     dbg_llui(expr)
#define dbg_ulli2(i1, i2)  dbg_llui2(i1, i2)

/* floats */
#define dbg_f(expr)        STMT( if (DBGLVL) { printf("%s " #expr " = %f\n", DEBUG_PROMPT, expr); fflush(stdout); } )
#define dbg_f2(f1, f2)     STMT( if (DBGLVL) { printf("%s " #f1 " = %f , " #f2 " = %f\n", DEBUG_PROMPT, f1, f2); fflush(stdout); } )

/* strings */
#define dbg_s(str)         STMT( if (DBGLVL) { printf("%s " #str " = \"%s\"\n", DEBUG_PROMPT, str); fflush(stdout); } )
#define dbg_s2(s1, s2)     STMT( if (DBGLVL) { printf("%s " #s1 " = \"%s\" , " #s2 " = \"%s\"\n", DEBUG_PROMPT, s1, s2); fflush(stdout); } )

/* boolean expressions */
#define dbg_b(expr)        STMT( if (DBGLVL) { printf("%s " #expr "  =>  %s\n", DEBUG_PROMPT, expr ? "[ TRUE ]" : "[ FALSE ]"); fflush(stdout); } )
#define dbg_b2(b1, b2)     STMT( if (DBGLVL) { printf("%s " #b1 " = %s , " #b2 " = %s\n", DEBUG_PROMPT, b1 ? "[ TRUE ]" : "[ FALSE ]", b2 ? "[ TRUE ]" : "[ FALSE ]"); fflush(stdout); } )

/* pointers */
#define dbg_p(expr)                                                               \
    STMT(                                                                      \
        if (expr==NULL)                                                        \
            printf("%s " #expr " = NULL\n", DEBUG_PROMPT);                     \
        else                                                                   \
            printf("%s " #expr " = %p\n", DEBUG_PROMPT, expr);                 \
        fflush(stdout);                                                        \
    )

/* print code point (e.g. in order to see after which point does an error occur) */
#define dbg()             STMT( if (DBGLVL) { printf("%s %s => %s:%d\n", DEBUG_PROMPT, __PRETTY_FUNCTION__, __FILE__, __LINE__); fflush(stdout); } )


#endif
