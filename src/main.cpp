#include "Global.h"
#include "KeyValueStore.h"
#include "ImmCompactionManager.h"
#include "GeomCompactionManager.h"
#include "LogCompactionManager.h"
#include "UrfCompactionManager.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <cstring>
#include <cassert>
#include <ctype.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <sys/time.h>

using namespace std;

#define CHECK_DUPLICATE_ARG(flag,opt) do {if (flag) { printf("Error: you have already set '-%c' argument\n", opt); exit(EXIT_FAILURE); }} while(0)

const uint64_t DEFAULT_INSERTBYTES = 1048576000LL; // 1GB
const size_t   DEFAULT_KEY_SIZE =             100; // 100 bytes
const size_t   DEFAULT_VALUE_SIZE =          1000; // 1000 bytes
const uint64_t DEFAULT_INSERTKEYS =  DEFAULT_INSERTBYTES / (DEFAULT_KEY_SIZE + DEFAULT_VALUE_SIZE);
const bool     DEFAULT_UNIQUE_KEYS =        false; // do not create unique keys

/*========================================================================
 *                             print_syntax
 *========================================================================*/
void print_syntax(char *progname)
{
     printf("syntax: %s -c compactionmanager [options]\n", progname);
     printf("        -c compactionmanager:   \"immediate\", \"geometric\", \"logarithmic\", \"urf\"\n");
     printf("        -r value:               r parameter, for geometric comp. manager only (default: %d)\n", DEFAULT_GEOM_R);
     printf("        -p value:               p parameter, for geometric comp. manager only (default: disabled)\n");
     printf("        -b blocksize:           block size in MB, for urf comp. manager only (default: %.0f)\n", b2mb(DEFAULT_URF_BLOCKSIZE));
     printf("        -f flushmem:            flush memory size in MB, for urf comp. manager only (default: 0,\n");
     printf("                                when memory is full flush only the biggest range)\n");
     printf("        -i insertbytes:         number of bytes to insert in MB (default: %.0f)\n", b2mb(DEFAULT_INSERTBYTES));
     printf("        -n numkeystoinsert:     number of keys to insert (default: %Ld)\n", DEFAULT_INSERTKEYS);
     printf("        -k keysize:             size of keys, in bytes (default: %d)\n", DEFAULT_KEY_SIZE);
     printf("        -v valuesize:           size of values, in bytes (default: %d)\n", DEFAULT_VALUE_SIZE);
     printf("        -u:                     create unique keys (default: %s)\n", (DEFAULT_UNIQUE_KEYS) ? "true " : "false");
     printf("        -m memory:              memory size in MB (default: %.0f)\n", b2mb(DEFAULT_MEMSTORE_SIZE));
//      printf("        -s:                     print periodic stats at stderr\n");
//      printf("        -o statsperiod:         print periodic stats every time that many MB are inserted\n");
//      printf("                                (default: memory size)\n");
     printf("        -h:                     print this help message and exit\n");
}

/*========================================================================
 *                               randstr
 *========================================================================*/
void randstr(char *s, const int len) {
    static const char alphanum[] =
//         "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    int size = sizeof(alphanum);

    for (int i = 0; i < len; i++) {
        s[i] = alphanum[rand() % (size - 1)];
    }

    s[len] = '\0';
}

/*========================================================================
 *                                main
 *========================================================================*/
int main(int argc, char **argv)
{
    int      cflag = 0,
             rflag = 0,
             pflag = 0,
             bflag = 0,
             fflag = 0,
             mflag = 0,
             iflag = 0,
             nflag = 0,
             kflag = 0,
             vflag = 0,
             uflag = 0,
             oflag = 0,
             sflag = 0,
             myopt,
             i,
             search_queries,
             geom_r,
             geom_p;
    uint64_t blocksize,
             flushmemorysize,
             memorysize,
             insertbytes,
             periodic_stats_step,
             num_keys_to_insert,
             timestamp,
             total_time;
    size_t   keysize,
             valuesize;
    char    *compmanager = NULL,
            *key = NULL,
            *value = NULL,
            *value2 = NULL;
    bool     periodic_stats_enabled,
             unique_keys;
    KeyValueStore *kvstore;
    struct timeval start, end;


    // We need at least compaction manager
    if (argc == 1) {
         print_syntax(argv[0]);
         exit(EXIT_FAILURE);
    }

    //==============================================================
    // get arguments
    //==============================================================
    while ((myopt = getopt (argc, argv, "hc:r:p:b:f:m:i:n:k:v:uo:s")) != -1) {
        switch (myopt)  {

        case 'h':
            print_syntax(argv[0]);
            exit(EXIT_SUCCESS);

        case 'c':
            CHECK_DUPLICATE_ARG(cflag, myopt);
            cflag = 1;
            compmanager = optarg;
            break;

        case 'r':
            CHECK_DUPLICATE_ARG(rflag, myopt);
            rflag = 1;
            geom_r = atoi(optarg);
            break;

        case 'p':
            CHECK_DUPLICATE_ARG(pflag, myopt);
            pflag = 1;
            geom_p = atoi(optarg);
            break;

        case 'b':
            CHECK_DUPLICATE_ARG(bflag, myopt);
            bflag = 1;
            blocksize = mb2b(atoll(optarg));
            break;

        case 'f':
            CHECK_DUPLICATE_ARG(fflag, myopt);
            fflag = 1;
            flushmemorysize = mb2b(atoll(optarg));
            break;

        case 'm':
            CHECK_DUPLICATE_ARG(mflag, myopt);
            mflag = 1;
            memorysize = mb2b(atoll(optarg));
            break;

        case 'i':
            CHECK_DUPLICATE_ARG(iflag, myopt);
            iflag = 1;
            insertbytes = mb2b(atof(optarg));
            break;

        case 'n':
            CHECK_DUPLICATE_ARG(nflag, myopt);
            nflag = 1;
            num_keys_to_insert = atoll(optarg);
            break;

        case 'k':
            CHECK_DUPLICATE_ARG(kflag, myopt);
            kflag = 1;
            keysize = atoi(optarg);
            break;

        case 'v':
            CHECK_DUPLICATE_ARG(vflag, myopt);
            vflag = 1;
            valuesize = atoi(optarg);
            break;

        case 'u':
            CHECK_DUPLICATE_ARG(uflag, myopt);
            uflag = 1;
            unique_keys = true;
            break;

        case 'o':
            CHECK_DUPLICATE_ARG(oflag, myopt);
            oflag = 1;
            periodic_stats_step = mb2b(atof(optarg));
            break;

        case 's':
            CHECK_DUPLICATE_ARG(sflag, myopt);
            sflag = 1;
            periodic_stats_enabled = true;
            break;

        case '?':
            if (optopt == 'c' || optopt == 'm' || optopt == 'i' || optopt == 'r'
                 || optopt == 'p' || optopt == 'b' || optopt == 'f') {
                fprintf (stderr, "Error: option -%c requires an argument.\n", optopt);
            } else if (isprint(optopt)) {
                fprintf (stderr, "Error: unknown option '-%c'.\n", optopt);
            } else {
                fprintf (stderr, "Error: unknown option character '\\x%x'.\n", optopt);
            }
            exit(EXIT_FAILURE);

        default:
            abort();
        }
    }

    for (i = optind; i < argc; i++) {
        printf ("Error: non-option argument: '%s'\n", argv[i]);
        exit(EXIT_FAILURE);
    }

    //==============================================================
    // set default values
    //==============================================================
    if (rflag == 0) {
        geom_r = DEFAULT_GEOM_R;
    }
    if (pflag == 0) {
        geom_p = DEFAULT_GEOM_P;
    }
    if (bflag == 0) {
        blocksize = DEFAULT_URF_BLOCKSIZE;
    }
    if (fflag == 0) {
        flushmemorysize = DEFAULT_URF_FLUSHMEMSIZE;
    }
    if (mflag == 0) {
        memorysize = DEFAULT_MEMSTORE_SIZE;
    }
    if (kflag == 0) {
        keysize = DEFAULT_KEY_SIZE;
    }
    if (vflag == 0) {
        valuesize = DEFAULT_VALUE_SIZE;
    }
    if (uflag == 0) {
        unique_keys = DEFAULT_UNIQUE_KEYS;
    }
    if (iflag == 0) {
        if (nflag == 0) {
            insertbytes = DEFAULT_INSERTBYTES;
        } else {
            insertbytes = num_keys_to_insert * (keysize + valuesize);
        }
    }
    if (nflag == 0) {
        num_keys_to_insert = insertbytes / (keysize + valuesize);
    }
    if (oflag == 0) {
        periodic_stats_step = memorysize;
    }
    if (sflag == 0) {
        periodic_stats_enabled = DEFAULT_STATS_ENABLED;
    }

    //==============================================================
    // check values
    //==============================================================
    if (!cflag) {
        printf("Error: you must set compaction manager\n");
        exit(EXIT_FAILURE);
    }
    if (cflag && strcmp(compmanager, "immediate") != 0 && strcmp(compmanager, "geometric") != 0
         && strcmp(compmanager, "logarithmic") != 0 && strcmp(compmanager, "urf") != 0) {
        printf("Error: compaction manager can be \"immediate\", \"geometric\", \"logarithmic\" or \"urf\"\n");
        exit(EXIT_FAILURE);
    }
    if (rflag && pflag && strcmp(compmanager, "geometric") == 0) {
        printf("Error: you cannot set both 'r' and 'p' parameters\n");
        exit(EXIT_FAILURE);
    }
    if (fflag && flushmemorysize > memorysize) {
        printf("Error: flush memory size cannot be greater than memory size\n");
        exit(EXIT_FAILURE);
    }
    if (kflag && keysize > MAX_KVTSIZE) {
        printf("Error: 'keysize' cannot be bigger than %lu\n", MAX_KVTSIZE);
        exit(EXIT_FAILURE);
    }
    if (vflag && valuesize > MAX_KVTSIZE) {
        printf("Error: 'valuesize' cannot be bigger than %lu\n", MAX_KVTSIZE);
        exit(EXIT_FAILURE);
    }
    if (nflag && iflag) {
        printf("Error: you cannot set both 'insertbytes' and 'numkeystoinsert' parameters\n");
        exit(EXIT_FAILURE);
    }

    //==============================================================
    // create keyvalue store and set parameter values
    //==============================================================
    // Immediate compaction manager
    if (strcmp(compmanager, "immediate") == 0) {
        kvstore = new KeyValueStore(KeyValueStore::IMM_CM);
    }
    // Geometric compaction manager
    else if (strcmp(compmanager, "geometric") == 0) {
        kvstore = new KeyValueStore(KeyValueStore::GEOM_CM);
        if (rflag) {
            ((GeomCompactionManager *)kvstore->get_compaction_manager())->set_R(geom_r);
        }
        if (pflag) {
            ((GeomCompactionManager *)kvstore->get_compaction_manager())->set_P(geom_p);
        }
    }
    // Logarithmic compaction manager
    else if (strcmp(compmanager, "logarithmic") == 0) {
        kvstore = new KeyValueStore(KeyValueStore::LOG_CM);
    }
    // URF compaction manager
    else if (strcmp(compmanager, "urf") == 0) {
        assert("not yet implemented" && 0);
        kvstore = new KeyValueStore(KeyValueStore::URF_CM);
        if (bflag)
            ((UrfCompactionManager *)kvstore->get_compaction_manager())->set_blocksize(blocksize);
        if (fflag)
            ((UrfCompactionManager *)kvstore->get_compaction_manager())->set_flushmem(flushmemorysize);
    }
    // other compaction manager?!
    else {
        printf("Error: unknown compaction manager (but we should not get here!)\n");
        exit(EXIT_FAILURE);
    }

    //==============================================================
    // print simulation parameters
    //==============================================================
    printf("# compaction_manager:  %15s\n", compmanager);
    printf("# memory_size:         %15.0f MB %s\n", b2mb(memorysize), (mflag == 0) ? "(default)" : "");
    printf("# insert_bytes:        %15.0f MB %s\n", b2mb(insertbytes), (iflag == 0) ? "(default)" : "");
    printf("# key_size:            %15d    %s\n", keysize, (kflag == 0) ? "(default)" : "");
    printf("# value_size:          %15d    %s\n", valuesize, (vflag == 0) ? "(default)" : "");
    printf("# keys_to_insert:      %15Ld\n", num_keys_to_insert);
    printf("# unique_keys:         %15s    %s\n", (unique_keys) ? "true" : "false", (uflag == 0) ? "(default)" : "");
    printf("# periodic_stats:      %15s    %s\n", (sflag) ? "true" : "false", (sflag == 0) ? "(default)" : "");
    if (sflag) {
         printf("# print_stats_every:   %15.0f MB %s\n", b2mb(periodic_stats_step), (oflag == 0) ? "(default)" : "");
    }
    if (strcmp(compmanager, "geometric") == 0) {
        if (pflag == 0) {
            printf("# geometric_r:         %15d    %s\n", geom_r, (rflag == 0) ? "(default)" : "");
        } else {
            printf("# geometric_p:         %15d    %s\n", geom_p, (pflag == 0) ? "(default, disabled)" : "");
        }
    } else if (strcmp(compmanager, "urf") == 0) {
        if (blocksize == 0) {
            printf("# urf_block_size:      %15s MB\n", "inf");
        } else {
            printf("# urf_block_size:      %15.0f MB %s\n", b2mb(blocksize), (bflag == 0) ? "(default)" : "");
        }
        printf("# urf_flushmem_size:   %15.0f MB %s\n", b2mb(flushmemorysize), (fflag == 0) ? "(default)" : "");
    }

    printf("# MBytes_INS  MBytes_RW   MBytes_W   MBytes_R   frag:AVG  CUR   IO_ops   IO_x  Tmerge  Sec_Passed\n");

    //==============================================================
    // execute puts and gets
    //==============================================================
    kvstore->set_memstore_maxsize(memorysize);
    key = (char *)malloc(MAX_KVTSIZE);
    value = (char *)malloc(MAX_KVTSIZE);

    for (uint64_t i = 0; i < num_keys_to_insert; i++) {
        randstr(key, keysize);
        randstr(value, valuesize);
        if (uflag) {
            sprintf(key, "%s%Ld", key, i); // make key unique
        }
        kvstore->put(key, value);

        // every 10000 puts perform a number of gets
        if (i && i % 10000 == 0) {
            system("echo 3 > /proc/sys/vm/drop_caches");
            total_time = 0;
            search_queries = 20; // if sq = 1000, and runsize = 100MB, then reading 1000 x 64KB = 64MB, many queries may be a cache hit
            for (int j = 0; j < search_queries; j++) {
                randstr(key, keysize);
                if (uflag) {
                    sprintf(key, "%s%Ld.%d", key, i, j); // make key unique
                }
                gettimeofday(&start, NULL);
                kvstore->get(key, &value2, &timestamp);
                gettimeofday(&end, NULL);
                total_time += (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
            }
            printf("i: %Ld diskruns: %d avg_searchms: %8.3f\n", i, kvstore->get_num_disk_files(), (total_time/ 100.0) / search_queries);
        }
    }

    free(key);
    free(value);
    delete kvstore;

    return EXIT_SUCCESS;
}
