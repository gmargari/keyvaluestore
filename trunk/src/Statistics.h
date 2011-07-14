#ifndef STATISTICS_H
#define STATISTICS_H

#include "Global.h"

#include <unistd.h>
#include <stdint.h>
#include <sys/time.h>

//==============================================================================
// stuct to hold global statistics
//==============================================================================

typedef struct {
    uint32_t gb;
    uint32_t mb;
    uint32_t kb;
    uint32_t b;
} byte_stats;

typedef struct {
    struct timeval start;
    struct timeval end;
    struct timeval total;
} time_stats;

typedef struct { // add new field -> update global_stats_init() and stats_sanity_check()
    byte_stats bytes_written;
    byte_stats bytes_read;
    uint32_t   num_writes;
    uint32_t   num_reads;
    time_stats read_time;
    time_stats write_time;
    time_stats compaction_time;
    time_stats total_time;
} keyvaluestore_stats;

//==============================================================================
// functions used to gather byte statistics
//==============================================================================

void        bytes_init(byte_stats *bytestats);
void        bytes_inc(byte_stats *bytestats, size_t bytes);
uint32_t    bytes_get_gb(byte_stats bytestats);
uint32_t    bytes_get_mb(byte_stats bytestats);
uint32_t    bytes_get_kb(byte_stats bytestats);
uint32_t    bytes_get_b(byte_stats bytestats);

//==============================================================================
// functions used to gather time statistics
//==============================================================================

void        time_init(time_stats *timestats);
void        time_start(time_stats *timestats);
void        time_end(time_stats *timestats);
time_t      time_get_secs(time_stats timestats);
suseconds_t time_get_usecs(time_stats timestats);

//==============================================================================
// functions used to gather integer statistics
//==============================================================================

void        num_init(uint32_t *numstats);
void        num_inc(uint32_t *numstats, int inc);

//==============================================================================
// functions to initialize, stats, enable and disable stats gather
//==============================================================================

void        global_stats_init();
void        global_stats_enable_gathering();
void        global_stats_disable_gathering();
void        stats_sanity_check();

//==============================================================================
// global variable holding stats
//==============================================================================

extern keyvaluestore_stats g_stats;

#endif
