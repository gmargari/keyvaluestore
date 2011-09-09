#ifndef GLOBAL_H
#define GLOBAL_H

// set debug level:
//  0 - debugging disabled
//  1 - normal mode     (assertions and sanity_check()s enabled)
//  2 - heavy debugging (assertions, sanity_checks()s and all other debugging functions enabled)
#define DBGLVL 0

//==============================================================================
// default values
//==============================================================================
#define DEFAULT_MEMSTORE_SIZE                   104857600LL // 100MB
#define MAX_INDEX_DIST                                65536 // 64KB
#define MAX_KVTSIZE          ((unsigned long)(1*1024*1024)) // 1MB (max size for key and for value)
#define MERGE_BUFSIZE        ((unsigned long)(4*1024*1024)) // 4MB (must be at least 2*MAX_KVTSIZE + 2*sizeof(uint64_t))
#define DEFAULT_STATS_ENABLED                         false
#define DEFAULT_STATS_STEP            DEFAULT_MEMSTORE_SIZE

//==============================================================================
// compaction managers' default values
//==============================================================================
#define DEFAULT_GEOM_R                                    3
#define DEFAULT_GEOM_P                                    0 // default disabled (no upper bound on number of partitions)
#define DEFAULT_URF_BLOCKSIZE                   268435456LL // 256MB
#define DEFAULT_URF_FLUSHMEMSIZE                        0LL // 0 -> flush biggest range only

//==============================================================================
// vfile-related defines
//==============================================================================
#define ROOT_DIR                    "/tmp/kvstore/"
#define DISKFILE_PREFIX             "dfile."
#define VFILE_PART_PREFIX           ".part"
#define VFILE_INFO_SUFFIX           ".info"
#define VFILE_INDEX_SUFFIX          ".index"
#define MAX_FILE_SIZE     2147483647LL // 2GB

//==============================================================================
// types, structs and enums
//==============================================================================

typedef enum { CM_MERGE_ONLINE, CM_MERGE_OFFLINE } merge_type;

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

//==============================================================================
// safe execution of macros
//==============================================================================

#define STMT( stuff )     do { stuff } while(0)

//==============================================================================
// macros to print an expression and its value
//==============================================================================

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
#define dbg()             STMT( if (DBGLVL) { printf("%s %s => %s:%d\n", DEBUG_PROMPT, __FUNCTION__, __FILE__, __LINE__); fflush(stdout); } )

//==============================================================================
// debugging level
//==============================================================================

#if DBGLVL == 0
    #define NDEBUG // disable assertions
#endif


#endif
