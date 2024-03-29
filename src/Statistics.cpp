// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#include "./Global.h"
#include "./Statistics.h"

#include <assert.h>
#include <iostream>
#include <iomanip>

using std::cout;
using std::endl;
using std::setw;
using std::right;

/* decleration of global variable holding stats */
keyvaluestore_stats g_stats;
static bool initialized = false;
static bool stats_gathering_enabled = true;

/*============================================================================
 *                                 bytes_init
 *============================================================================*/
void bytes_init(byte_stats *bytestats) {
    assert(initialized);
    if (stats_gathering_enabled == false) {
        return;
    }

    bytestats->gb = 0;
    bytestats->mb = 0;
    bytestats->kb = 0;
    bytestats->b = 0;
}

/*============================================================================
 *                                 bytes_inc
 *============================================================================*/
void bytes_inc(byte_stats *bytestats, size_t bytes) {
    assert(initialized);
    if (stats_gathering_enabled == false) {
        return;
    }

    bytestats->b += bytes % (1 << 10);
    bytes = bytes >> 10;
    if (bytestats->b > (1 >> 10)) {
        bytestats->kb += bytestats->b >> 10;
        bytestats->b = bytestats->b % (1 << 10);
    }

    bytestats->kb += bytes % (1 << 10);
    bytes = bytes >> 10;
    if (bytestats->kb > (1 >> 10)) {
        bytestats->mb += bytestats->kb >> 10;
        bytestats->kb = bytestats->kb % (1 << 10);
    }

    bytestats->mb += bytes % (1 << 10);
    bytes = bytes >> 10;
    if (bytestats->mb > (1 >> 10)) {
        bytestats->gb += bytestats->mb >> 10;
        bytestats->mb = bytestats->mb % (1 << 10);
    }

    bytestats->gb += bytes % (1 << 10);
}

/*============================================================================
 *                               bytes_get_gb
 *============================================================================*/
uint32_t bytes_get_gb(byte_stats bytestats) {
    assert(initialized);
    return bytestats.gb;
}

/*============================================================================
 *                               bytes_get_mb
 *============================================================================*/
uint32_t bytes_get_mb(byte_stats bytestats) {
    assert(initialized);
    return bytestats.mb;
}

/*============================================================================
 *                               bytes_get_kb
 *============================================================================*/
uint32_t bytes_get_kb(byte_stats bytestats) {
    assert(initialized);
    return bytestats.kb;
}

/*============================================================================
 *                               bytes_get_b
 *============================================================================*/
uint32_t bytes_get_b(byte_stats bytestats) {
    assert(initialized);
    return bytestats.b;
}

/*============================================================================
 *                                 time_init
 *============================================================================*/
void time_init(time_stats *timestats) {
    assert(initialized);
    if (stats_gathering_enabled == false) {
        return;
    }

    timestats->total.tv_sec = 0;
    timestats->total.tv_usec = 0;
}

/*============================================================================
 *                                 time_start
 *============================================================================*/
void time_start(time_stats *timestats) {
    assert(initialized);
    if (stats_gathering_enabled == false) {
        return;
    }

    gettimeofday(&(timestats->start), NULL);
}

/*============================================================================
 *                                 time_end
 *============================================================================*/
void time_end(time_stats *timestats) {
    struct timeval newtime;

    assert(initialized);
    if (stats_gathering_enabled == false) {
        return;
    }

    gettimeofday(&(timestats->end), NULL);
    newtime.tv_sec = timestats->end.tv_sec - timestats->start.tv_sec;
    newtime.tv_usec = timestats->end.tv_usec - timestats->start.tv_usec;
    timestats->total.tv_sec += newtime.tv_sec;
    timestats->total.tv_usec += newtime.tv_usec;
    if (timestats->total.tv_usec >= 1000000) {
        timestats->total.tv_sec += 1;
        timestats->total.tv_usec -= 1000000;
    }
}

/*============================================================================
 *                              time_get_secs
 *============================================================================*/
uint32_t time_get_secs(time_stats timestats) {
    assert(initialized);
    return timestats.total.tv_sec;
}

/*============================================================================
 *                              time_get_usecs
 *============================================================================*/
uint32_t time_get_usecs(time_stats timestats) {
    assert(initialized);
    return timestats.total.tv_usec;
}

/*============================================================================
 *                                 num_init
 *============================================================================*/
void num_init(uint32_t *numstats) {
    assert(initialized);
    if (stats_gathering_enabled == false) {
        return;
    }

    (*numstats) = 0;
}

/*============================================================================
 *                                  num_inc
 *============================================================================*/
void num_inc(uint32_t *numstats, int inc) {
    assert(initialized);
    if (stats_gathering_enabled == false) {
        return;
    }

    (*numstats) += inc;
}

/*============================================================================
 *                             global_stats_init
 *============================================================================*/
void global_stats_init() {
    assert(initialized == false);
    initialized = true;

    bytes_init(&(g_stats.bytes_inserted));
    bytes_init(&(g_stats.bytes_written));
    bytes_init(&(g_stats.bytes_read));
    num_init(&(g_stats.num_writes));
    num_init(&(g_stats.num_reads));
    time_init(&(g_stats.total_time));
    time_init(&(g_stats.compaction_time));
    time_init(&(g_stats.read_time));
    time_init(&(g_stats.write_time));
    time_init(&(g_stats.merge_time));
    time_init(&(g_stats.free_time));
    g_stats.disk_files = 0;

    time_start(&(g_stats.total_time));
}

/*============================================================================
 *                       global_stats_enable_gathering
 *============================================================================*/
void global_stats_enable_gathering() {
    stats_gathering_enabled = true;
}

/*============================================================================
 *                       global_stats_disable_gathering
 *============================================================================*/
void global_stats_disable_gathering() {
    stats_gathering_enabled = false;
}

/*============================================================================
 *                             stats_sanity_check
 *============================================================================*/
void stats_sanity_check() {
//     assert(time_get_secs(g_stats.total_time) >=
//       time_get_secs(g_stats.compaction_time));
//     assert(time_get_secs(g_stats.compaction_time) >=
//       time_get_secs(g_stats.merge_time) + time_get_secs(g_stats.free_time));
//     assert(time_get_secs(g_stats.merge_time) >=
//       time_get_secs(g_stats.read_time) + time_get_secs(g_stats.write_time));
}

/*============================================================================
 *                             print_stats_header
 *============================================================================*/
void print_stats_header() {
    cout << "# mb_ins |  Ttotal Tcompac    Tput |  Tmerge   Tfree Tcmrest | "
         << "   Tmem   Tread  Twrite | mb_read mb_writ    reads   writes | runs"
         << endl;
}

/*============================================================================
 *                                 print_stats
 *============================================================================*/
void print_stats() {
    int32_t ttime;

    time_end(&(g_stats.total_time));
    time_start(&(g_stats.total_time));

    // mb inserted
    cout << setw(8)  << right << gb2mb(bytes_get_gb(g_stats.bytes_inserted))
                                   + bytes_get_mb(g_stats.bytes_inserted);
    // total time
    cout << setw(10) << right << time_get_secs(g_stats.total_time);
    // compaction time
    cout << setw(8)  << right << time_get_secs(g_stats.compaction_time);
    // put time (total - compaction)
    ttime = time_get_secs(g_stats.total_time)
              - time_get_secs(g_stats.compaction_time);
    cout << setw(8)  << right << (ttime < 0 ? 0 : ttime);
    // merge time
    cout << setw(10) << right << time_get_secs(g_stats.merge_time);
    // free time
    cout << setw(8)  << right << time_get_secs(g_stats.free_time);
    // compaction rest time (compaction - merge - free)
    ttime = time_get_secs(g_stats.compaction_time)
              - time_get_secs(g_stats.merge_time)
              - time_get_secs(g_stats.free_time);
    cout << setw(8)  << right << (ttime < 0 ? 0 : ttime);
    // memory time (merge - read - write)
    ttime = time_get_secs(g_stats.merge_time)
              - time_get_secs(g_stats.read_time)
              - time_get_secs(g_stats.write_time);
    cout << setw(10) << right << (ttime < 0 ? 0 : ttime);
    // read time
    cout << setw(8)  << right << time_get_secs(g_stats.read_time);
    // write time
    cout << setw(8)  << right << time_get_secs(g_stats.write_time);
    // bytes read
    cout << setw(10) << right << gb2mb(bytes_get_gb(g_stats.bytes_read))
                                   + bytes_get_mb(g_stats.bytes_read);
    // bytes written
    cout << setw(8)  << right << gb2mb(bytes_get_gb(g_stats.bytes_written))
                                   + bytes_get_mb(g_stats.bytes_written);
    // read operations
    cout << setw(9)  << right << g_stats.num_reads;
    // write operations
    cout << setw(9)  << right << g_stats.num_writes;
    // number of disk files
    cout << setw(7)  << right << g_stats.disk_files;
    cout << endl;
}
