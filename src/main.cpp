// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#include "./Global.h"

#include <math.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <getopt.h>
#include <sys/time.h>
#include <signal.h>
#include <limits.h>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "./KeyValueStore.h"
#include "./ImmCompactionManager.h"
#include "./GeomCompactionManager.h"
#include "./LogCompactionManager.h"
#include "./RangemergeCompactionManager.h"
#include "./CassandraCompactionManager.h"
#include "./Statistics.h"
#include "./Scanner.h"
#include "./RequestThrottle.h"

using std::cout;
using std::cerr;
using std::endl;
using std::setw;
using std::right;
using std::flush;

//------------------------------------------------------------------------------
// forward declaration of functions
//------------------------------------------------------------------------------

void  *put_routine(void *args);
void  *get_routine(void *args);
void   print_put_get_stats(int signum);
// call once to initialize. next calls return time difference from first call
uint64_t get_cur_time();
void   randstr_r(char *s, const int len, uint32_t *seed);
void   zipfstr_r(char *s, const int len, double zipf_param, uint32_t *seed);
void   orderedstr_r(char *s, const int len, uint32_t *seed);
int    zipf_r(double zipf_param, uint32_t *seed);
void   check_duplicate_arg_and_set(int *flag, int opt);
int    numdigits(uint64_t num);

//------------------------------------------------------------------------------
// default values
//------------------------------------------------------------------------------

const uint64_t DEFAULT_INSERTBYTES = 1048576000LL;  // 1GB
const uint32_t DEFAULT_KEY_SIZE =             100;  // 100 bytes
const uint32_t DEFAULT_VALUE_SIZE =          1000;  // 1000 bytes
const uint64_t DEFAULT_INSERTKEYS =  DEFAULT_INSERTBYTES /
                                     (DEFAULT_KEY_SIZE + DEFAULT_VALUE_SIZE);
const bool     DEFAULT_UNIQUE_KEYS =        false;
const bool     DEFAULT_ZIPF_KEYS =          false;
const bool     DEFAULT_ORDERED_KEYS =       false;
const int      DEFAULT_NUM_GET_THREADS =        0;
const int      DEFAULT_PUT_THRPUT =             0;  // req per sec, 0: disable
const int      DEFAULT_GET_THRPUT =            10;  // req per sec, 0: disable
const int      DEFAULT_RANGE_GET_SIZE =        10;  // get 10 KVs per range get
const bool     DEFAULT_FLUSH_PCACHE =       false;
const bool     DEFAULT_STATS_PRINT =        false;
const int      DEFAULT_STATS_PRINT_INTERVAL =   5;  // print stats every 5 sec
const int32_t  ZIPF_MAX_NUM =             1000000;

struct thread_args {
    int tid;
    int sflag;
    int uflag;
    int zipf_keys;
    double zipf_param;
    int ordered_keys;
    double ordered_prob;
    int print_kv_and_continue;
    uint64_t num_keys_to_insert;
    uint32_t keysize;
    uint32_t valuesize;
    int put_thrput;
    int get_thrput;
    int range_get_size;
    bool flush_page_cache;
    bool print_periodic_stats;
    KeyValueStore *kvstore;
};

bool put_thread_finished = false;
uint32_t *gets_count = NULL;
uint32_t *gets_latency = NULL;
int gets_num_threads = 0;
uint32_t puts_count = 0;
uint32_t puts_latency = 0;

/*============================================================================
 *                             print_syntax
 *============================================================================*/
void print_syntax(char *progname) {
    cout << "syntax: " << progname << " -c compactionmanager [options]" << endl;
    cout << endl;
    cout << "COMPACTION MANAGER" << endl;
    cout << " -c, --compaction-manager VALUE      'nomerge', 'immediate', 'geometric', 'logarithmic'," << endl;
    cout << "                                     'rangemerge' or 'cassandra'" << endl;
    cout << " -r, --geometric-r VALUE             Geometric R parameter [" << DEFAULT_GEOM_R << "]" << endl;
    cout << " -p, --geometric-p VALUE             Geometric P parameter [disabled]" << endl;
    cout << " -b, --rangemerge-blocksize VALUE    Rangemerge block size, in MB [" << b2mb(DEFAULT_RNG_BLOCKSIZE) << "]" << endl;
    cout << " -l, --cassandra-l VALUE             Cassandra L parameter [" << DEFAULT_CASS_K << "]" << endl;
    cout << " -m, --memstore-size VALUE           memstore size, in MB [" << b2mb(DEFAULT_MEMSTORE_SIZE) << "]" << endl;
    cout << endl;
    cout << "PUT" << endl;
    cout << " -i, --insert-bytes VALUE            number of bytes to insert in MB [" << b2mb(DEFAULT_INSERTBYTES) << "]" << endl;
    cout << " -n, --num-keys VALUE                number of KVs to insert [" << DEFAULT_INSERTKEYS << "]" << endl;
    cout << " -k, --key-size VALUE                size of keys, in bytes [" << DEFAULT_KEY_SIZE << "]" << endl;
    cout << " -v, --value-size VALUE              size of values, in bytes [" << DEFAULT_VALUE_SIZE << "]" << endl;
    cout << " -u, --unique-keys                   create unique keys [" << (DEFAULT_UNIQUE_KEYS ? "true" : "false") << "]" << endl;
    cout << " -z, --zipf-keys VALUE               create zipfian keys, with given distribution parameter [" << (DEFAULT_ZIPF_KEYS ? "true" : "false") << "]" << endl;
    cout << " -o, --ordered-keys VALUE            keys created are ordered, with VALUE probability being random [" << (DEFAULT_ORDERED_KEYS ? "true" : "false") << "]" << endl;
    cout << " -P, --put-throughput VALUE          put requests per sec (0: unlimited) [" << DEFAULT_PUT_THRPUT << "]" << endl;
    cout << endl;
    cout << "GET" << endl;
    cout << " -g, --get-threads VALUE             number of get threads [" << DEFAULT_NUM_GET_THREADS << "]" << endl;
    cout << " -G, --get-throughput VALUE          get requests per sec per thread (0: unlimited) [" << DEFAULT_GET_THRPUT << "]" << endl;
    cout << " -R, --range-get-size VALUE          max number of KVs to read (0: point get) [" << DEFAULT_RANGE_GET_SIZE << "]" << endl;
    cout << " -x, --flush-page-cache              flush page cache after each compaction [" << (DEFAULT_FLUSH_PCACHE ? "true" : "false") << "]" << endl;
    cout << endl;
    cout << "VARIOUS" << endl;
    cout << " -e, --print-kvs-to-stdout           print KVs that would be inserted and exit" << endl;
    cout << " -s, --read-kvs-from-stdin           read KVs from stdin" << endl;
    cout << " -t, --print-periodic-stats          print stats on stderr about compactions and gets every " << DEFAULT_STATS_PRINT_INTERVAL << " sec" << endl;
    cout << " -h, --help                          print this help message and exit" << endl;
}

/*============================================================================
 *                                main
 *============================================================================*/
int main(int argc, char **argv) {
    const char short_args[] = "c:r:p:b:l:m:i:n:k:v:uz:o:P:g:G:R:xesth";
    const struct option long_opts[] = {
             {"compaction-manager",   required_argument,  0, 'c'},
             {"geometric-r",          required_argument,  0, 'r'},
             {"geometric-p",          required_argument,  0, 'p'},
             {"rangemerge-blocksize", required_argument,  0, 'b'},
             {"cassandra-l",          required_argument,  0, 'l'},
             {"memstore-size",        required_argument,  0, 'm'},
             {"insert-bytes",         required_argument,  0, 'i'},
             {"num-keys",             required_argument,  0, 'n'},
             {"key-size",             required_argument,  0, 'k'},
             {"value-size",           required_argument,  0, 'v'},
             {"unique-keys",          no_argument,        0, 'u'},
             {"zipf-keys",            required_argument,  0, 'z'},
             {"ordered-keys",         required_argument,  0, 'o'},
             {"put-throughput",       required_argument,  0, 'P'},
             {"get-threads",          required_argument,  0, 'g'},
             {"get-throughput",       required_argument,  0, 'G'},
             {"range-get-size",       required_argument,  0, 'R'},
             {"flush-page-cache",     no_argument,        0, 'x'},
             {"print-kvs-to-stdout",  no_argument,        0, 'e'},
             {"read-kvs-from-stdin",  no_argument,        0, 's'},
             {"print-periodic-stats", no_argument,        0, 't'},
             {"help",                 no_argument,        0, 'h'},
             {0, 0, 0, 0}
    };
    int      cflag = 0,
             rflag = 0,
             pflag = 0,
             bflag = 0,
             lflag = 0,
             mflag = 0,
             iflag = 0,
             nflag = 0,
             kflag = 0,
             vflag = 0,
             uflag = 0,
             Pflag = 0,
             zflag = 0,
             oflag = 0,
             gflag = 0,
             Gflag = 0,
             Rflag = 0,
             xflag = 0,
             eflag = 0,
             sflag = 0,
             tflag = 0,
             myopt,
             i,
             geom_r,
             geom_p,
             cass_l,
             num_get_threads,
             retval,
             indexptr,
             put_thrput,
             get_thrput,
             range_get_size;
    uint64_t blocksize,
             memstore_size,
             insertbytes,
             num_keys_to_insert;
    uint32_t keysize,
             valuesize;
    double   zipf_param,
             ordered_prob;
    char    *cmanager = NULL,
            *key = NULL,
            *value = NULL,
            *end_key = NULL;
    bool     unique_keys,
             zipf_keys,
             ordered_keys,
             print_kv_and_continue = false,
             flush_page_cache,
             print_periodic_stats;
    KeyValueStore *kvstore;
    struct thread_args *targs;
    pthread_t *thread;
    struct tm *current;
    time_t now;

    // we need at least compaction manager
    if (argc == 1) {
         print_syntax(argv[0]);
         exit(EXIT_FAILURE);
    }

    //--------------------------------------------------------------------------
    // get arguments
    //--------------------------------------------------------------------------
    while ((myopt = getopt_long(argc, argv, short_args, long_opts, &indexptr))
              != -1) {
        switch (myopt)  {
            case 'h':
                print_syntax(argv[0]);
                exit(EXIT_SUCCESS);

            case 'c':
                check_duplicate_arg_and_set(&cflag, myopt);
                cmanager = optarg;
                break;

            case 'r':
                check_duplicate_arg_and_set(&rflag, myopt);
                geom_r = atoi(optarg);
                break;

            case 'p':
                check_duplicate_arg_and_set(&pflag, myopt);
                geom_p = atoi(optarg);
                break;

            case 'b':
                check_duplicate_arg_and_set(&bflag, myopt);
                blocksize = mb2b(atoll(optarg));
                break;

            case 'l':
                check_duplicate_arg_and_set(&lflag, myopt);
                cass_l = atoi(optarg);
                break;

            case 'm':
                check_duplicate_arg_and_set(&mflag, myopt);
                memstore_size = mb2b(atoll(optarg));
                break;

            case 'i':
                check_duplicate_arg_and_set(&iflag, myopt);
                insertbytes = mb2b(atof(optarg));
                break;

            case 'n':
                check_duplicate_arg_and_set(&nflag, myopt);
                num_keys_to_insert = atoll(optarg);
                break;

            case 'k':
                check_duplicate_arg_and_set(&kflag, myopt);
                keysize = atoi(optarg);
                break;

            case 'v':
                check_duplicate_arg_and_set(&vflag, myopt);
                valuesize = atoi(optarg);
                break;

            case 'u':
                check_duplicate_arg_and_set(&uflag, myopt);
                unique_keys = true;
                break;

            case 'z':
                check_duplicate_arg_and_set(&zflag, myopt);
                zipf_keys = true;
                zipf_param = atof(optarg);
                break;

            case 'o':
                check_duplicate_arg_and_set(&oflag, myopt);
                ordered_keys = true;
                ordered_prob = atof(optarg);
                break;

            case 'P':
                check_duplicate_arg_and_set(&Pflag, myopt);
                put_thrput = atoi(optarg);
                break;

            case 'g':
                check_duplicate_arg_and_set(&gflag, myopt);
                num_get_threads = atoi(optarg);
                break;

            case 'G':
                check_duplicate_arg_and_set(&Gflag, myopt);
                get_thrput = atoi(optarg);
                break;

            case 'R':
                check_duplicate_arg_and_set(&Rflag, myopt);
                range_get_size = atoi(optarg);
                break;

            case 'x':
                check_duplicate_arg_and_set(&xflag, myopt);
                flush_page_cache = true;
                break;

            case 'e':
                check_duplicate_arg_and_set(&eflag, myopt);
                print_kv_and_continue = true;
                break;

            case 's':
                check_duplicate_arg_and_set(&sflag, myopt);
                break;

            case 't':
                check_duplicate_arg_and_set(&tflag, myopt);
                print_periodic_stats = true;
                break;

            case '?':
                exit(EXIT_FAILURE);

            default:
                abort();
        }
    }

    for (i = optind; i < argc; i++) {
        cerr << "Error: non-option argument: '" << argv[i] << "'" << endl;
        exit(EXIT_FAILURE);
    }

    //--------------------------------------------------------------------------
    // set default values
    //--------------------------------------------------------------------------
    if (rflag == 0) {
        geom_r = DEFAULT_GEOM_R;
    }
    if (pflag == 0) {
        geom_p = DEFAULT_GEOM_P;
    }
    if (bflag == 0) {
        blocksize = DEFAULT_RNG_BLOCKSIZE;
    }
    if (lflag == 0) {
        cass_l = DEFAULT_CASS_K;
    }
    if (mflag == 0) {
        memstore_size = DEFAULT_MEMSTORE_SIZE;
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
    if (zflag == 0) {
        zipf_keys = DEFAULT_ZIPF_KEYS;
    }
    if (oflag == 0) {
        ordered_keys = DEFAULT_ORDERED_KEYS;
    }
    if (Pflag == 0) {
        put_thrput = DEFAULT_PUT_THRPUT;
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
    if (gflag == 0) {
        num_get_threads = DEFAULT_NUM_GET_THREADS;
    }
    if (Gflag == 0) {
        get_thrput = DEFAULT_GET_THRPUT;
    }
    if (Rflag == 0) {
        range_get_size = DEFAULT_RANGE_GET_SIZE;
    }
    if (xflag == 0) {
        flush_page_cache = DEFAULT_FLUSH_PCACHE;
    }
    if (tflag == 0) {
        print_periodic_stats = DEFAULT_STATS_PRINT;
    }

    //--------------------------------------------------------------------------
    // check values
    //--------------------------------------------------------------------------
    if (!cflag) {
        cerr << "Error: you must set compaction manager" << endl;
        exit(EXIT_FAILURE);
    }
    if (cflag && strcmp(cmanager, "nomerge") && strcmp(cmanager, "immediate")
          && strcmp(cmanager, "geometric") && strcmp(cmanager, "logarithmic")
          && strcmp(cmanager, "rangemerge") && strcmp(cmanager, "cassandra")) {
        cerr << "Error: compaction manager can be 'nomerge', 'immediate', ";
        cerr << "'geometric', 'logarithmic', 'rangemerge' or 'cassandra'";
        cerr << endl;
        exit(EXIT_FAILURE);
    }
    if (rflag && pflag && strcmp(cmanager, "geometric") == 0) {
        cerr << "Error: you cannot set both 'r' and 'p' parameters" << endl;
        exit(EXIT_FAILURE);
    }
    if (kflag && keysize > MAX_KEY_SIZE) {
        cerr << "Error: 'keysize' cannot be bigger than " << MAX_KEY_SIZE << endl;
        exit(EXIT_FAILURE);
    }
    if (vflag && valuesize > MAX_VALUE_SIZE) {
        cerr << "Error: 'valuesize' cannot be bigger than " << MAX_VALUE_SIZE << endl;
        exit(EXIT_FAILURE);
    }
    if (zflag && zipf_param <= 0) {
        cerr << "Error: zipf parameter must be > 0" << endl;
        exit(EXIT_FAILURE);
    }
    if (oflag && (ordered_prob < 0 || ordered_prob > 1)) {
        cerr << "Error: probability for ordered keys must be in [0, 1]" << endl;
        exit(EXIT_FAILURE);
    }
    if (nflag && iflag) {
        cerr << "Error: you cannot set both 'insertbytes' and 'numkeystoinsert' parameters" << endl;
        exit(EXIT_FAILURE);
    }
    if (sflag) {
        if (kflag) {
            cerr << "Ignoring '-k' flag (keysize): keys will be read from stdin" << endl;
            kflag = 0;
            keysize = DEFAULT_KEY_SIZE;
        }
        if (vflag) {
            cerr << "Ignoring '-v' flag (valuesize): values will be read from stdin" << endl;
            vflag = 0;
            valuesize = DEFAULT_VALUE_SIZE;
        }
        if (uflag) {
            cerr << "Ignoring '-u' flag (unique keys): keys will be read from stdin" << endl;
            uflag = 0;
            unique_keys = DEFAULT_UNIQUE_KEYS;
        }
        if (zflag) {
            cerr << "Ignoring '-z' flag (zipf keys): keys will be read from stdin" << endl;
            zflag = 0;
            zipf_keys = DEFAULT_ZIPF_KEYS;
        }
    }

    //--------------------------------------------------------------------------
    // create keyvalue store and set parameter values
    //--------------------------------------------------------------------------
    // Null compaction manager
    if (strcmp(cmanager, "nomerge") == 0) {
        kvstore = new KeyValueStore(KeyValueStore::NOMERGE_CM);
    }
    // Immediate compaction manager
    else if (strcmp(cmanager, "immediate") == 0) {
        kvstore = new KeyValueStore(KeyValueStore::IMM_CM);
    }
    // Geometric compaction manager
    else if (strcmp(cmanager, "geometric") == 0) {
        kvstore = new KeyValueStore(KeyValueStore::GEOM_CM);
        if (rflag) {
            ((GeomCompactionManager *)kvstore->get_compaction_manager())->set_R(geom_r);
        }
        if (pflag) {
            ((GeomCompactionManager *)kvstore->get_compaction_manager())->set_P(geom_p);
        }
    }
    // Logarithmic compaction manager
    else if (strcmp(cmanager, "logarithmic") == 0) {
        kvstore = new KeyValueStore(KeyValueStore::LOG_CM);
    }
    // Rangemerge compaction manager
    else if (strcmp(cmanager, "rangemerge") == 0) {
        kvstore = new KeyValueStore(KeyValueStore::RNGMERGE_CM);
        if (bflag) {
            ((RangemergeCompactionManager *)kvstore->get_compaction_manager())->set_blocksize(blocksize);
        }
    }
    // Cassandra compaction manager
    else if (strcmp(cmanager, "cassandra") == 0) {
        kvstore = new KeyValueStore(KeyValueStore::CASSANDRA_CM);
        if (lflag) {
            ((CassandraCompactionManager *)kvstore->get_compaction_manager())->set_L(cass_l);
        }
    } else {
        cerr << "Error: unknown compaction manager (but we should not get here!)" << endl;
        exit(EXIT_FAILURE);
    }

    //--------------------------------------------------------------------------
    // print values of parameters
    //--------------------------------------------------------------------------
    cout << "# compaction_manager:  " << setw(15) << cmanager << endl;
    cout << "# memstore_size:       " << setw(15) << b2mb(memstore_size) << " MB    " << (mflag == 0 ? "(default)" : "") << endl;
    if (sflag) {
        cout << "# insert_bytes:        " << setw(15) << "?       (keys and values will be read from stdin)" << endl;
        cout << "# key_size:            " << setw(15) << "?       (keys and values will be read from stdin)" << endl;
        cout << "# value_size:          " << setw(15) << "?       (keys and values will be read from stdin)" << endl;
        cout << "# keys_to_insert:      " << setw(15) << "?       (keys and values will be read from stdin)" << endl;
        cout << "# unique_keys:         " << setw(15) << "?       (keys and values will be read from stdin)" << endl;
        cout << "# zipf_keys:           " << setw(15) << "?       (keys and values will be read from stdin)" << endl;
        cout << "# ordered_keys:        " << setw(15) << "?       (keys and values will be read from stdin)" << endl;
    } else {
        cout << "# insert_bytes:        " << setw(15) << b2mb(insertbytes) << " MB    " << (iflag == 0 ? "(default)" : "") << endl;
        cout << "# key_size:            " << setw(15) << keysize << " B     " << (kflag == 0 ? "(default)" : "") << endl;
        cout << "# value_size:          " << setw(15) << valuesize << " B     " << (vflag == 0 ? "(default)" : "") << endl;
        cout << "# keys_to_insert:      " << setw(15) << num_keys_to_insert << endl;
        cout << "# unique_keys:         " << setw(15) << (unique_keys ? "true" : "false") << "       " << (uflag == 0 ? "(default)" : "") << endl;
        cout << "# zipf_keys:           " << setw(15) << (zipf_keys ? "true" : "false") << "       " << (zflag == 0 ? "(default)" : "") << endl;
        if (zipf_keys) {
            cout << "# zipf_parameter:      " << setw(15) << zipf_param << endl;
        }
        cout << "# ordered_keys:        " << setw(15) << (ordered_keys ? "true" : "false") << "       " << (oflag == 0 ? "(default)" : "") << endl;
        if (ordered_keys) {
            cout << "# ordered_prob:        " << setw(15) << ordered_prob << endl;
        }
    }
    cout << "# put_throughput:      " << setw(15) << put_thrput << " req/s " << (Pflag == 0 ? "(default)" : "") << endl;
    cout << "# get_threads:         " << setw(15) << num_get_threads << "       " << (gflag == 0 ? "(default)" : "") << endl;
    cout << "# get_throughput:      " << setw(15) << get_thrput << " req/s " << (Gflag == 0 ? "(default)" : "") << endl;
    cout << "# get_type:            " << setw(15) << (range_get_size ? "range" : "point") << endl;
    if (range_get_size) {
        cout << "# range_get_size:      " << setw(15) << range_get_size << " keys  " << (Rflag == 0 ? "(default)" : "") << endl;
    }
    if (strcmp(cmanager, "geometric") == 0) {
        if (pflag == 0) {
            cout << "# geometric_r:         " << setw(15) << geom_r << "       " << (rflag == 0 ? "(default)" : "") << endl;
        } else {
            cout << "# geometric_p:         " << setw(15) << geom_p << "       " << (pflag == 0 ? "(default, disabled)" : "") << endl;
        }
    } else if (strcmp(cmanager, "rangemerge") == 0) {
        if (blocksize == 0) {
            cout << "# rngmerge_block_size: " << setw(15) << "inf" << " MB   " << endl;
        } else {
            cout << "# rngmerge_block_size: " << setw(15) << b2mb(blocksize) << " MB    " << (bflag == 0 ? "(default)" : "") << endl;
        }
    }
    if (strcmp(cmanager, "cassandra") == 0) {
        cout << "# cassandra_l:         " << setw(15) << cass_l << "       " << (lflag == 0 ? "(default)" : "") << endl;
    }
    cout << "# read_from_stdin:     " << setw(15) << (sflag ? "true" : "false") << "       " << (sflag == 0 ? "(default)" : "") << endl;
    cout << "# flush_page_cache:    " << setw(15) << (flush_page_cache ? "true" : "false") << "       " << (xflag == 0 ? "(default)" : "") << endl;
    cout << "# print_periodic_stats: " << setw(14) << (print_periodic_stats ? "true" : "false") << "       " << (tflag == 0 ? "(default)" : "") << endl;
    cout << "# debug_level:         " << setw(15) << DBGLVL << endl;
    cout << "# mergebuf_size:       " << setw(15) << MERGEBUF_SIZE << " B" << endl;

    time(&now);
    current = localtime(&now);
    cerr << "[DATE]      " << current->tm_mday << "/" << current->tm_mon + 1 << "/" << current->tm_year + 1900 << endl;
    cerr << "[TIME]      " << current->tm_hour << ":" << current->tm_min << ":" << current->tm_sec << endl;

    fflush(stdout);
//    system("svn info | grep Revision | awk '{printf \"# svn_revision:   %20d\\n\", $2}'");
    print_stats_header();

    //--------------------------------------------------------------------------
    // initialize variables
    //--------------------------------------------------------------------------
    kvstore->set_memstore_maxsize(memstore_size);
    key = (char *)malloc(MAX_KEY_SIZE + 1);
    end_key = (char *)malloc(MAX_KEY_SIZE + 1);
    value = (char *)malloc(MAX_VALUE_SIZE + 1);

    //--------------------------------------------------------------------------
    // fill-in arguments of put thread and get threads
    //--------------------------------------------------------------------------
    targs = (struct thread_args *) malloc((1 + num_get_threads) *
                                          sizeof(thread_args));
    for (i = 0; i < 1 + num_get_threads; i++) {
        targs[i].tid = i;
        targs[i].uflag = uflag;
        targs[i].sflag = sflag;
        targs[i].zipf_keys = zipf_keys;
        targs[i].zipf_param = zipf_param;
        targs[i].ordered_keys = ordered_keys;
        targs[i].ordered_prob = ordered_prob;
        targs[i].print_kv_and_continue = print_kv_and_continue;
        targs[i].num_keys_to_insert = num_keys_to_insert;
        targs[i].keysize = keysize,
        targs[i].valuesize = valuesize;
        targs[i].kvstore = kvstore;
        targs[i].put_thrput = put_thrput;
        targs[i].get_thrput = get_thrput;
        targs[i].range_get_size = range_get_size;
        targs[i].flush_page_cache = flush_page_cache;
        targs[i].print_periodic_stats = print_periodic_stats;
    }

    //--------------------------------------------------------------------------
    // set signal handler and timer for periodic printing of get latency/thrput
    //--------------------------------------------------------------------------
    if (print_periodic_stats) {
        struct itimerval timer;

        gets_num_threads = num_get_threads;
        gets_count = (uint32_t *)malloc(sizeof(uint32_t) * gets_num_threads);
        gets_latency = (uint32_t *)malloc(sizeof(uint32_t) * gets_num_threads);
        for (i = 0; i < gets_num_threads; i++) {
            gets_count[i] = 0;
            gets_latency[i] = 0;
        }

        if (signal(SIGALRM, print_put_get_stats) == SIG_ERR) {
            perror("Could not set signal handler");
            exit(EXIT_FAILURE);
        }

        timer.it_interval.tv_sec = DEFAULT_STATS_PRINT_INTERVAL;
        timer.it_interval.tv_usec = 0;
        timer.it_value.tv_sec = timer.it_interval.tv_sec;
        timer.it_value.tv_usec = timer.it_interval.tv_usec;
        if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
            perror("Could not set timer");
            exit(EXIT_FAILURE);
        }

        get_cur_time(); // call once to initialize
    }

    //--------------------------------------------------------------------------
    // create put thread and get threads
    //--------------------------------------------------------------------------
    thread = (pthread_t *)malloc((1 + num_get_threads) * sizeof(pthread_t));
    for (i = 0; i < 1 + num_get_threads; i++) {
        if (i == 0) {
            retval = pthread_create(&thread[i], NULL, put_routine, (void *)&targs[i]);
        } else {
            retval = pthread_create(&thread[i], NULL, get_routine, (void *)&targs[i]);
        }
        if (retval) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    //--------------------------------------------------------------------------
    // wait for threads to finish
    //--------------------------------------------------------------------------
    for (i = 0; i < 1 + num_get_threads; i++) {
        pthread_join(thread[i], NULL);
    }

    if (!print_kv_and_continue) {  // if we did insert some kvs to kvstore

        // if we crete unique keys, check num keys in store == num keys inserted
        if (uflag) {
            assert(kvstore->get_num_mem_keys() + kvstore->get_num_disk_keys()
                     == num_keys_to_insert);
        }

        // flush remaining memory tuples
        while (kvstore->get_mem_size()) {
            kvstore->request_flush();
        }

        if (uflag) {
            assert(kvstore->get_num_mem_keys() == 0);
            assert(kvstore->get_num_disk_keys() == num_keys_to_insert);
        }
    }

    time(&now);
    current = localtime(&now);
    cerr << "[DATE]      " << current->tm_mday << "/" << current->tm_mon + 1 << "/" << current->tm_year + 1900 << endl;
    cerr << "[TIME]      " << current->tm_hour << ":" << current->tm_min << ":" << current->tm_sec << endl;

    free(gets_count);
    free(gets_latency);
    free(key);
    free(end_key);
    free(value);
    free(thread);
    free(targs);
    delete kvstore;

    return EXIT_SUCCESS;
}

/*============================================================================
 *                              put_routine
 *============================================================================*/
void *put_routine(void *args) {
    struct thread_args *targs = (struct thread_args *)args;
    int      uflag = targs->uflag,
             sflag = targs->sflag,
             zipf_keys = targs->zipf_keys,
             ordered_keys = targs->ordered_keys,
             print_kv_and_continue = targs->print_kv_and_continue,
             compaction_occured = 0;
    double   zipf_param = targs->zipf_param,
             ordered_prob = targs->ordered_prob;
    bool     flush_page_cache = targs->flush_page_cache,
             print_periodic_stats = targs->print_periodic_stats;
    uint64_t num_keys_to_insert = targs->num_keys_to_insert;
    uint32_t keysize = targs->keysize,
             valuesize = targs->valuesize;
    KeyValueStore *kvstore = targs->kvstore;
    uint32_t kseed = targs->tid,  // kseed = getpid() + time(NULL);
             vseed = kseed + 1,
             sseed = 0,
             pseed = 0;
    char    *key = NULL,
            *value = NULL;
    uint32_t keylen, valuelen;
    uint64_t bytes_inserted = 0;
    RequestThrottle throttler(targs->put_thrput);
    std::ostringstream buf;
    struct timeval start, end;

    if (DBGLVL > 0) {
        cout << "# [DEBUG]   put thread started" << endl;
    }

    key = (char *)malloc(MAX_KEY_SIZE + 1);
    value = (char *)malloc(MAX_VALUE_SIZE + 1);

    // if we read keys and values from stdin set num_keys_to_insert to infinity
    if (sflag) {
        num_keys_to_insert = -1;  // ('num_keys_to_insert' is uint64_t)
    }

    //--------------------------------------------------------------------------
    // until we have inserted all keys
    //--------------------------------------------------------------------------
    for (uint64_t i = 0; i < num_keys_to_insert; i++) {

        //--------------------------------------------------------------
        // throttle request rate
        //--------------------------------------------------------------
        throttler.throttle();

        if (sflag) {
            //------------------------------------------------------------------
            // read key and value from stdin
            //------------------------------------------------------------------
            if (scanf("%s %s", key, value) != 2) {
                break;
            }
            keylen = strlen(key);
            valuelen = strlen(value);
        } else {
            //------------------------------------------------------------------
            // create a random key and a random value
            //------------------------------------------------------------------
            if (zipf_keys) {
                zipfstr_r(key, keysize, zipf_param, &kseed);
                keylen = keysize;
            } else if (ordered_keys) {
                // with probability 'ordered_prob', create a random key. else,
                // create an ordered key
                if ((float)rand_r(&pseed) / (float)RAND_MAX < ordered_prob) {
                    randstr_r(key, keysize, &kseed);
                } else {
                    orderedstr_r(key, keysize, &sseed);
                }
                keylen = keysize;
            } else {
                randstr_r(key, keysize, &kseed);
                keylen = keysize;
            }
            if (uflag) {
                // TODO: sprintf(key, "%s", key) -> undefined behaviour!
                sprintf(key, "%s.%Lu", key, i);  // make unique
                keylen += 1 + numdigits(i);
            }
            randstr_r(value, valuesize, &vseed);
            valuelen = valuesize;
        }

        //----------------------------------------------------------------------
        // just print <key, value> to stdout, do not insert into kvstore
        //----------------------------------------------------------------------
        if (print_kv_and_continue) {
            cout << key << " " << value << endl;
            continue;
        }

        // if this put will trigger a compaction, print stats before compaction
        if (kvstore->memstore_will_fill(key, keylen, value, valuelen)) {
            print_stats();
            compaction_occured = 1;

            if (print_periodic_stats) {
                buf.str("");
                buf << "[FLUSH_MEM] " << setw(9) << right << get_cur_time() << " START "
                    << kvstore->get_num_disk_files() << endl;
                cerr << buf.str() << flush;
            }
        }

        //----------------------------------------------------------------------
        // insert <key, value> into store
        //----------------------------------------------------------------------
        if (print_periodic_stats) {
            gettimeofday(&start, NULL);
        }

        kvstore->put(key, keylen, value, valuelen);
        bytes_inserted += keylen + valuelen;

        if (print_periodic_stats) {
            gettimeofday(&end, NULL);
            puts_count++;
            puts_latency += (end.tv_sec - start.tv_sec)*1000 +
                            (end.tv_usec - start.tv_usec)/1000;
        }

        // if this put triggered a compaction, print stats after compaction
        if (compaction_occured) {
            print_stats();
            compaction_occured = 0;

            // flush page cache after compaction, so next get()s will go to disk
            if (flush_page_cache) {
                if (print_periodic_stats) {
                    buf.str("");
                    buf << "[DROP_CACH] " << setw(9) << right << get_cur_time() << " START" << endl;
                    cerr << buf.str() << flush;
                }

                system("sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'");

                if (print_periodic_stats) {
                    buf.str("");
                    buf << "[DROP_CACH] " << setw(9) << right << get_cur_time() << " END" << endl;
                    cerr << buf.str() << flush;
                }
            }

            if (print_periodic_stats) {
                buf.str("");
                buf << "[FLUSH_MEM] " << setw(9) << right << get_cur_time() << " END "
                    << kvstore->get_num_disk_files() << endl;
                cerr << buf.str() << flush;
            }
        }
    }

    if (DBGLVL > 0) {
        cout << "# [DEBUG]   put thread finished" << endl;
    }

    put_thread_finished = true;

    free(key);
    free(value);
    pthread_exit(NULL);
}

/*============================================================================
 *                              get_routine
 *============================================================================*/
void *get_routine(void *args) {
    struct thread_args *targs = (struct thread_args *)args;
    int      uflag = targs->uflag,
             zipf_keys = targs->zipf_keys,
             range_get_size = targs->range_get_size;
    double   zipf_param = targs->zipf_param;
    bool     print_periodic_stats = targs->print_periodic_stats;
    uint32_t keysize = targs->keysize, keylen;
    uint32_t kseed = targs->tid;  // kseed = getpid() + time(NULL);
    char     key[MAX_KEY_SIZE + 1];
    int      i = -1;
    Scanner *scanner = new Scanner(targs->kvstore);
    RequestThrottle throttler(targs->get_thrput);
    struct timeval start, end;

    if (DBGLVL > 0) {
        cout << "# [DEBUG]   get thread " << targs->tid << " started" << endl;
    }

    while (!put_thread_finished) {

        //--------------------------------------------------------------
        // throttle request rate
        //--------------------------------------------------------------
        throttler.throttle();

        //--------------------------------------------------------------
        // create a random key
        //--------------------------------------------------------------
        if (zipf_keys) {
            zipfstr_r(key, keysize, zipf_param, &kseed);
            keylen = keysize;
        } else {
            randstr_r(key, keysize, &kseed);
            keylen = keysize;
        }
        if (uflag) {
            // TODO: sprintf(key, "%s", key) -> undefined behaviour!
            sprintf(key, "%s#%d", key, ++i);
            keylen += 1 + numdigits(i);
        }

        //--------------------------------------------------------------
        // execute range get() or point get()
        //--------------------------------------------------------------
        if (print_periodic_stats) {
            gettimeofday(&start, NULL);
        }

        if (range_get_size == 0) {
            scanner->point_get(key, keylen);
        } else {
            scanner->range_get(key, keylen, NULL, 0, range_get_size);
        }

        if (print_periodic_stats) {
            gettimeofday(&end, NULL);
            gets_count[targs->tid - 1]++;
            gets_latency[targs->tid - 1] += (end.tv_sec - start.tv_sec)*1000 +
                                            (end.tv_usec - start.tv_usec)/1000;
        }
    }

    if (DBGLVL > 0) {
        cout << "# [DEBUG]   get thread " << targs->tid << " finished" << endl;
    }

    delete scanner;
    pthread_exit(NULL);
}

/*============================================================================
 *                        print_put_get_stats
 *============================================================================*/
void print_put_get_stats(int signum) {
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static uint64_t gets_count_old = 0, gets_latency_old = 0, old_time = 0;
    static uint64_t puts_count_old = 0, puts_latency_old = 0;
    uint64_t gets_count_cur, gets_latency_cur, cur_time;
    uint64_t puts_count_cur, puts_latency_cur;
    float sec_lapsed;
    std::ostringstream buf;

    // needed in some architectures
    if (signal(SIGALRM, print_put_get_stats) == SIG_ERR) {
        perror("Could not set signal handler");
        exit(EXIT_FAILURE);
    }

    // if mutex locked, signal was caught while last signal handling is not
    // completed: return immediately (we'll catch next signal)
    if (pthread_mutex_trylock(&mutex) != 0) {
        return;
    }

    gets_count_cur = 0;
    gets_latency_cur = 0;
    for (int i = 0; i < gets_num_threads; i++) {
        gets_count_cur += gets_count[i];
        gets_latency_cur += gets_latency[i];
    }
    gets_count_cur -= gets_count_old;
    gets_latency_cur -= gets_latency_old;
    gets_count_old += gets_count_cur;
    gets_latency_old += gets_latency_cur;

    puts_count_cur = puts_count - puts_count_old;
    puts_latency_cur = puts_latency - puts_latency_old;
    puts_count_old += puts_count_cur;
    puts_latency_old += puts_latency_cur;

    cur_time = get_cur_time();
    if (old_time == 0) {
        sec_lapsed = DEFAULT_STATS_PRINT_INTERVAL;
    } else {
        sec_lapsed = (cur_time - old_time) / 1000.0;
    }
    old_time = cur_time;

    buf.str("");
    if (gets_num_threads) {
        buf << "[GET_STATS] " << setw(9) << right << cur_time << " "
            << sec_lapsed << " " << gets_count_cur << " " << gets_latency_cur << endl;
    }
    buf << "[PUT_STATS] " << setw(9) << right << cur_time << " "
        << sec_lapsed << " " << puts_count_cur << " " << puts_latency_cur << endl;
    cerr << buf.str() << flush;

    pthread_mutex_unlock(&mutex);
}

/*============================================================================
 *                            get_cur_time
 *============================================================================*/
uint64_t get_cur_time() {
    static struct timeval start;
    struct timeval now;
    static int first_time = 1;

    if (first_time) {
        first_time = 0;
        gettimeofday(&start, NULL);
        return 0;
    }

    gettimeofday(&now, NULL);
    return (now.tv_sec - start.tv_sec)*1000 + (now.tv_usec - start.tv_usec)/1000;
}

/*============================================================================
 *                              randstr_r
 *============================================================================*/
void randstr_r(char *s, const int len, uint32_t *seed) {
    static const char alphanum[] =
//         "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    int size = sizeof(alphanum);

    assert(len >= 0);

    for (int i = 0; i < len; i++) {
        s[i] = alphanum[rand_r(seed) % (size - 1)];
    }

    s[len] = '\0';
}

/*============================================================================
 *                              zipfstr_r
 *============================================================================*/
void zipfstr_r(char *s, const int len, double zipf_param, uint32_t *seed) {
    static int num_digits = log10(ZIPF_MAX_NUM) + 1;
    static char key_prefix[MAX_KEY_SIZE + 1];
    static bool first = true;

    if (first) {
        first = false;
        // key prefix must be common for all keys to follow zipf distribution
        randstr_r(key_prefix, len - num_digits, seed);
    }

    sprintf(s, "%s%0*d", key_prefix, num_digits, zipf_r(zipf_param, seed));
}

/*============================================================================
 *                             orderedstr_r
 *============================================================================*/
void orderedstr_r(char *s, const int len, uint32_t *seed) {
    static int num_digits = log10(ULONG_MAX) + 1;
    static char key_prefix[MAX_KEY_SIZE + 1];
    static bool first = true;

    if (first) {
        first = false;
        // key prefix must be common for all keys
        randstr_r(key_prefix, len - num_digits, seed);
    }

    sprintf(s, "%s%0*lu", key_prefix, num_digits, (*seed)++);
}

/*
 * code below from:
 *    http://www.csee.usf.edu/~christen/tools/toolpage.html
 * code was modified to precompute sum of probabilities and use integers
 * instead of doubles
 */

/*============================================================================
 *                               zipf_r
 *============================================================================*/
int zipf_r(double zipf_param, uint32_t *seed) {
    static int *sum_prob = NULL;  // sum of probabilities
    int z,                        // uniform random number (0 <= z <= RAND_MAX)
        zipf_value,               // computed exponential value to be returned
        i,
        first, last, mid;         // for binary search

    // compute sum of probabilities on first call only
    if (sum_prob == NULL) {
        double *sum_prob_f;
        double c = 0;             // normalization constant

        for (i = 1; i <= ZIPF_MAX_NUM; i++) {
            c = c + (1.0 / pow((double) i, zipf_param));
        }
        c = 1.0 / c;

        // precompute sum of probabilities
        sum_prob_f = (double *)malloc((ZIPF_MAX_NUM + 1) * sizeof(*sum_prob_f));
        sum_prob_f[0] = 0;
        for (i = 1; i <= ZIPF_MAX_NUM; i++) {
            sum_prob_f[i] = sum_prob_f[i-1] + c / pow((double) i, zipf_param);
        }

        // from array of doubles sum_prob_f[] that contains values in range
        // [0,1], compute array of integers sum_prob_i[] that contains values
        // in range [0,RAND_MAX]
        sum_prob = (int *)malloc((ZIPF_MAX_NUM + 1) * sizeof(*sum_prob));
        for (i = 0; i <= ZIPF_MAX_NUM; i++) {
            sum_prob[i] = (int)(sum_prob_f[i] * RAND_MAX);
        }
    }

    // pull a uniform random number (0 <= z <= RAND_MAX)
    z = rand_r(seed);

    // map z to the value (find the first 'i' for which sum_prob[i] >= z)
    first = 1;
    last = ZIPF_MAX_NUM;
    while (first <= last) {
        mid = (last - first)/2 + first;  // avoid overflow
        if (z > sum_prob[mid]) {
            first = mid + 1;
        } else if (z < sum_prob[mid]) {
            last = mid - 1;
        } else {
            break;
        }
    }

    if (sum_prob[mid] >= z) {
        zipf_value = mid;
    } else {
        zipf_value = mid + 1;
    }

    // assert that zipf_value is between 1 and N
    assert((zipf_value >= 1) && (zipf_value <= ZIPF_MAX_NUM));

    return (zipf_value);
}

/*============================================================================
 *                        check_duplicate_arg_and_set
 *============================================================================*/
void check_duplicate_arg_and_set(int *flag, int opt) {
    if (*flag) {
        cerr << "Error: you have already set '-" << (char)opt << "' argument" << endl;
        exit(EXIT_FAILURE);
    }
    *flag = 1;
}

/*============================================================================
 *                             numdigits
 *============================================================================*/
int numdigits(uint64_t num) {
    int digits = 0;

    if (num < 10) return 1;
    if (num < 100) return 2;
    if (num < 1000) return 3;
    if (num < 10000) return 4;
    if (num < 100000) return 5;
    if (num < 1000000) return 6;
    if (num < 10000000) return 7;
    if (num < 100000000) return 8;
    if (num < 1000000000) return 9;

    do {
        num /= 10;
        ++digits;
    } while (num > 0);

    return digits;
}
