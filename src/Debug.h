// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_DEBUG_H_
#define SRC_DEBUG_H_

#define STMT(stuff)      do { stuff } while (0)
#define DBGPRINT(stuff) STMT( if (DBGLVL) { stuff; fflush(stdout); } )

//==============================================================================
// macros to print an expression and its value
//==============================================================================

#if DBGLVL > 0
    #include <stdio.h>  // for functions below
#endif

#if DBGLVL == 0
    #define NDEBUG      // disable assertions
#endif

#define DEBUG_PROMPT "# [DEBUG]  "

/* integers */
#define dbg_i(expr)        DBGPRINT(printf("%s " #expr " = %d\n", DEBUG_PROMPT, expr); )
#define dbg_i2(i1, i2)     DBGPRINT(printf("%s " #i1 " = %d , " #i2 " = %d\n", DEBUG_PROMPT, i1, i2); )

/* long integers */
#define dbg_li(expr)       DBGPRINT(printf("%s " #expr " = %ld\n", DEBUG_PROMPT, expr); )
#define dbg_li2(i1, i2)    DBGPRINT(printf("%s " #i1 " = %ld , " #i2 " = %ld\n", DEBUG_PROMPT, i1, i2); )
#define dbg_l(expr)        dbg_li(expr)
#define dbg_l2(i1, i2)     dbg_li2(i1, i2)

/* unsigned integers */
#define dbg_ui(expr)       DBGPRINT(printf("%s " #expr " = %u\n", DEBUG_PROMPT, expr); )
#define dbg_ui2(i1, i2)    DBGPRINT(printf("%s " #i1 " = %u , " #i2 " = %u\n", DEBUG_PROMPT, i1, i2); )
#define dbg_u(expr)        dbg_ui(expr)
#define dbg_u2(i1, i2)     dbg_ui2(i1, i2)

/* long unsigned integers */
#define dbg_lui(expr)      DBGPRINT(printf("%s " #expr " = %lu\n", DEBUG_PROMPT, expr); )
#define dbg_lui2(i1, i2)   DBGPRINT(printf("%s " #i1 " = %lu , " #i2 " = %lu\n", DEBUG_PROMPT, i1, i2); )
#define dbg_uli(expr)      dbg_lui(expr)
#define dbg_ul(expr)       dbg_lui(expr)
#define dbg_uli2(i1, i2)   dbg_lui2(i1, i2)

/* long long unsigned integers */
#define dbg_lli(expr)      DBGPRINT(printf("%s " #expr " = %Ld\n", DEBUG_PROMPT, expr); )
#define dbg_lli2(i1, i2)   DBGPRINT(printf("%s " #i1 " = %Ld , " #i2 " = %Ld\n", DEBUG_PROMPT, i1, i2); )

/* long long unsigned integers */
#define dbg_llui(expr)     DBGPRINT(printf("%s " #expr " = %Lu\n", DEBUG_PROMPT, expr); )
#define dbg_llui2(i1, i2)  DBGPRINT(printf("%s " #i1 " = %Lu , " #i2 " = %Lu\n", DEBUG_PROMPT, i1, i2); )
#define dbg_ulli(expr)     dbg_llui(expr)
#define dbg_ulli2(i1, i2)  dbg_llui2(i1, i2)

/* floats */
#define dbg_f(expr)        DBGPRINT(printf("%s " #expr " = %f\n", DEBUG_PROMPT, expr); )
#define dbg_f2(f1, f2)     DBGPRINT(printf("%s " #f1 " = %f , " #f2 " = %f\n", DEBUG_PROMPT, f1, f2); )

/* strings */
#define dbg_s(str)         DBGPRINT(printf("%s " #str " = \"%s\"\n", DEBUG_PROMPT, str); )
#define dbg_s2(s1, s2)     DBGPRINT(printf("%s " #s1 " = \"%s\" , " #s2 " = \"%s\"\n", DEBUG_PROMPT, s1, s2); )

/* boolean expressions */
#define dbg_b(expr)        DBGPRINT(printf("%s " #expr "  =>  %s\n", DEBUG_PROMPT, expr ? "[ TRUE ]" : "[ FALSE ]"); )
#define dbg_b2(b1, b2)     DBGPRINT(printf("%s " #b1 " = %s , " #b2 " = %s\n", DEBUG_PROMPT, b1 ? "[ TRUE ]" : "[ FALSE ]", b2 ? "[ TRUE ]" : "[ FALSE ]"); )

/* pointers */
#define dbg_p(expr)                                             \
    DBGPRINT(                                                   \
        if (expr == NULL)                                       \
            printf("%s " #expr " = NULL\n", DEBUG_PROMPT);      \
        else                                                    \
            printf("%s " #expr " = %p\n", DEBUG_PROMPT, expr);  \
    )

/* print code point (e.g. in order to see after which point does an error occur) */
#define dbg()             DBGPRINT(printf("%s %s => %s:%d\n", DEBUG_PROMPT, __FUNCTION__, __FILE__, __LINE__); )

#define my_perror_exit()                                                  \
    STMT(                                                                 \
        printf("Error: %s => %s:%d\n", __FUNCTION__, __FILE__, __LINE__); \
        perror("");                                                       \
        exit(EXIT_FAILURE);                                               \
    )

#define my_exit()                                                         \
    STMT(                                                                 \
        printf("Error: %s => %s:%d\n", __FUNCTION__, __FILE__, __LINE__); \
        exit(EXIT_FAILURE);                                               \
    )

//==============================================================================
// debugging level
//==============================================================================

#endif  // SRC_DEBUG_H_
