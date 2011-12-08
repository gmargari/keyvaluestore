#include "Global.h"
#include "KeyValueStore.h"
#include "ImmCompactionManager.h"
#include "GeomCompactionManager.h"
#include "LogCompactionManager.h"
#include "RangemergeCompactionManager.h"
#include "CassandraCompactionManager.h"
#include "Statistics.h"
#include "Scanner.h"
#include "RequestThrottle.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <cstring>
#include <cassert>
#include <ctype.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <sys/time.h>
#include <pthread.h>
#include <getopt.h>

using namespace std;

#define CHECK_DUPLICATE_ARG(flag,opt) do {if (flag) { cout << "Error: you have already set '-" << opt << "' argument" << endl; exit(EXIT_FAILURE); }} while(0)

//------------------------------------------------------------------------------
// forward declaration of functions
//------------------------------------------------------------------------------

void  *put_routine(void *args);
void  *get_routine(void *args);
void   randstr_r(char *s, const int len, uint32_t *seed);
void   zipfstr_r(char *s, const int len, uint32_t *seed);
int    zipf_r(uint32_t *seed);

//------------------------------------------------------------------------------
// default values
//------------------------------------------------------------------------------

const uint64_t DEFAULT_INSERTBYTES = 1048576000LL; // 1GB
const uint32_t DEFAULT_KEY_SIZE =             100; // 100 bytes
const uint32_t DEFAULT_VALUE_SIZE =          1000; // 1000 bytes
const uint64_t DEFAULT_INSERTKEYS =  DEFAULT_INSERTBYTES / (DEFAULT_KEY_SIZE + DEFAULT_VALUE_SIZE);
const bool     DEFAULT_UNIQUE_KEYS =        false;
const bool     DEFAULT_ZIPF_KEYS =          false;
const int      DEFAULT_NUM_GET_THREADS =        0;
const int      DEFAULT_PUT_THRPUT =             0; // req per second, 0: disable throttling
const int      DEFAULT_GET_THRPUT =            10; // req per second, 0: disable throttling

double zipf_alpha = 0.9;       // parameter for zipf() function
long int zipf_n = 1000000;     // parameter for zipf() function
// 'zipf_n' affects the generation time of a random zipf number: there
// is a loop in zipf() that goes from 1 to 'n'. So, the greater 'n' is,
// the more time zipf() needs to generate a zipf number.

struct thread_args {
    int tid;
    int sflag;
    int uflag;
    int zipf_keys;
    int print_kv_and_continue;
    uint64_t num_keys_to_insert;
    uint32_t keysize;
    uint32_t valuesize;
    int put_thrput;
    int get_thrput;
    KeyValueStore *kvstore;
};

bool put_thread_finished = false;

/*============================================================================
 *                             print_syntax
 *============================================================================*/
void print_syntax(char *progname)
{
     cout << "syntax: " << progname << " -c compactionmanager [options]" << endl;
     cout << endl;
     cout << "COMPACTION MANAGER" << endl;
     cout << " -c, --compaction-manager [VALUE]    \"nomerge\", \"immediate\", \"geometric\", \"logarithmic\", \"rangemerge\", \"cassandra\"" << endl;
     cout << " -r, --geometric-r [VALUE]           R parameter (default: " << DEFAULT_GEOM_R << ")" << endl;
     cout << " -p, --geometric-p [VALUE]           P parameter (default: disabled)" << endl;
     cout << " -b, --rangemerge-blocksize [VALUE]  block size in MB (default: " << b2mb(DEFAULT_RNGMERGE_BLOCKSIZE) << ")" << endl;
     cout << " -f, --rangemerge-flushmem [VALUE]   flush memory size in MB (default: 0, flush only the biggest range)" << endl;
     cout << " -l, --cassandra-l [VALUE]           L parameter (default: " << DEFAULT_CASS_K << ")" << endl;
     cout << " -m, --memorysize [VALUE]            memory size in MB (default: " << b2mb(DEFAULT_MEMSTORE_SIZE) << ")" << endl;
     cout << endl;
     cout << "PUT" << endl;
     cout << " -i, --insert-bytes [VALUE]          number of bytes to insert in MB (default: " << b2mb(DEFAULT_INSERTBYTES) << ")" << endl;
     cout << " -n, --num-keys [VALUE]              number of keys to insert (default: " << DEFAULT_INSERTKEYS << ")" << endl;
     cout << " -k, --key-size [VALUE]              size of keys, in bytes (default: " << DEFAULT_KEY_SIZE << ")" << endl;
     cout << " -v, --value-size [VALUE]            size of values, in bytes (default: " << DEFAULT_VALUE_SIZE << ")" << endl;
     cout << " -u, --unique-keys                   create unique keys (default: " << (DEFAULT_UNIQUE_KEYS ? "true " : "false") << ")" << endl;
     cout << " -z, --zipf-keys                     create zipfian keys (default: false, uniform keys)" << endl;
     cout << " -P, --put-throughput [VALUE]        throttle requests per sec (0 unlimited, default: " << DEFAULT_PUT_THRPUT << ")" << endl;
     cout << endl;
     cout << "GET" << endl;
     cout << " -g, --get-threads [VALUE]           number of get threads (default: " << DEFAULT_NUM_GET_THREADS << ")" << endl;
     cout << " -G, --get-throughput [VALUE]        throttle requests per sec per thread (0 unlimited, default: " << DEFAULT_GET_THRPUT << ")" << endl;
     cout << endl;
     cout << "VARIOUS" << endl;
     cout << " -e, --print-kvs-to-stdout           print key-values that would be inserted and exit" << endl;
     cout << " -s, --read-kvs-from-stdin           read key-values from stdin" << endl;
     cout << " -h, --help                          print this help message and exit" << endl;
}

/*============================================================================
 *                                main
 *============================================================================*/
int main(int argc, char **argv)
{
    const char short_args[] = "c:r:p:b:f:l:m:i:n:k:v:uzP:g:G:esh";
    const struct option long_opts[] = {
             {"compaction-manager",   required_argument,  0, 'c'},
             {"geometric-r",          required_argument,  0, 'r'},
             {"geometric-p",          required_argument,  0, 'p'},
             {"rangemerge-blocksize", required_argument,  0, 'b'},
             {"rangemerge-flushmem",  required_argument,  0, 'f'},
             {"cassandra-l",          required_argument,  0, 'l'},
             {"memory-size",          required_argument,  0, 'm'},
             {"insert-bytes",         required_argument,  0, 'i'},
             {"num-keys",             required_argument,  0, 'n'},
             {"key-size",             required_argument,  0, 'k'},
             {"value-size",           required_argument,  0, 'v'},
             {"unique-keys",          no_argument,        0, 'u'},
             {"zipf-keys",            no_argument,        0, 'z'},
             {"put-throughput",       required_argument,  0, 'P'},
             {"get-threads",          required_argument,  0, 'g'},
             {"get-throughput",       required_argument,  0, 'G'},
             {"print-kvs-to-stdout",  no_argument,        0, 'e'},
             {"read-kvs-from-stdin",  no_argument,        0, 's'},
             {"help",                 no_argument,        0, 'h'},
             {0, 0, 0, 0}
    };
    int      cflag = 0,
             rflag = 0,
             pflag = 0,
             bflag = 0,
             fflag = 0,
             lflag = 0,
             mflag = 0,
             iflag = 0,
             nflag = 0,
             kflag = 0,
             vflag = 0,
             uflag = 0,
             Pflag = 0,
             zflag = 0,
             gflag = 0,
             Gflag = 0,
             sflag = 0,
             eflag = 0,
             myopt,
             i,
             geom_r,
             geom_p,
             cass_l,
             num_get_threads,
             retval,
             indexptr,
             put_thrput,
             get_thrput;
    uint64_t blocksize,
             flushmemorysize,
             memorysize,
             insertbytes,
             num_keys_to_insert;
    uint32_t keysize,
             valuesize;
    char    *compmanager = NULL,
            *key = NULL,
            *value = NULL,
            *end_key = NULL;
    bool     unique_keys,
             zipf_keys,
             print_kv_and_continue = false;
    KeyValueStore *kvstore;
    struct thread_args *targs;
    pthread_t *thread;

    // we need at least compaction manager
    if (argc == 1) {
         print_syntax(argv[0]);
         exit(EXIT_FAILURE);
    }

    //--------------------------------------------------------------------------
    // get arguments
    //--------------------------------------------------------------------------
    while ((myopt = getopt_long(argc, argv, short_args, long_opts, &indexptr)) != -1) {
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

        case 'l':
            CHECK_DUPLICATE_ARG(lflag, myopt);
            lflag = 1;
            cass_l = atoi(optarg);
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

        case 'z':
            CHECK_DUPLICATE_ARG(zflag, myopt);
            zflag = 1;
            zipf_keys = true;
            break;

        case 'P':
            CHECK_DUPLICATE_ARG(Pflag, myopt);
            Pflag = 1;
            put_thrput = atoi(optarg);
            break;

        case 'g':
            CHECK_DUPLICATE_ARG(gflag, myopt);
            gflag = 1;
            num_get_threads = atoi(optarg);
            break;

        case 'G':
            CHECK_DUPLICATE_ARG(Gflag, myopt);
            Gflag = 1;
            get_thrput = atoi(optarg);
            break;

        case 'e':
            CHECK_DUPLICATE_ARG(eflag, myopt);
            eflag = 1;
            print_kv_and_continue = true;
            break;

        case 's':
            CHECK_DUPLICATE_ARG(sflag, myopt);
            sflag = 1;
            break;

        case '?':
            exit(EXIT_FAILURE);

        default:
            abort();
        }
    }

    for (i = optind; i < argc; i++) {
        cout << "Error: non-option argument: '" << argv[i] << "'" << endl;
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
        blocksize = DEFAULT_RNGMERGE_BLOCKSIZE;
    }
    if (fflag == 0) {
        flushmemorysize = DEFAULT_RNGMERGE_FLUSHMEMSIZE;
    }
    if (lflag == 0) {
        cass_l = DEFAULT_CASS_K;
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
    if (zflag == 0) {
        zipf_keys = DEFAULT_ZIPF_KEYS;
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

    //--------------------------------------------------------------------------
    // check values
    //--------------------------------------------------------------------------
    if (!cflag) {
        cout << "Error: you must set compaction manager" << endl;
        exit(EXIT_FAILURE);
    }
    if (cflag && strcmp(compmanager, "nomerge") && strcmp(compmanager, "immediate") != 0
         && strcmp(compmanager, "geometric") != 0 && strcmp(compmanager, "logarithmic") != 0
         && strcmp(compmanager, "rangemerge") != 0 && strcmp(compmanager, "cassandra") != 0) {
        cout << "Error: compaction manager can be \"nomerge\", \"immediate\", \"geometric\", \"logarithmic\" or \"rangemerge\" or \"cassandra\"" << endl;
        exit(EXIT_FAILURE);
    }
    if (rflag && pflag && strcmp(compmanager, "geometric") == 0) {
        cout << "Error: you cannot set both 'r' and 'p' parameters" << endl;
        exit(EXIT_FAILURE);
    }
    if (fflag && flushmemorysize > memorysize) {
        cout << "Error: flush memory size cannot be greater than memory size" << endl;
        exit(EXIT_FAILURE);
    }
    if (kflag && keysize > MAX_KVTSIZE) {
        cout << "Error: 'keysize' cannot be bigger than " << MAX_KVTSIZE << endl;
        exit(EXIT_FAILURE);
    }
    if (vflag && valuesize > MAX_KVTSIZE) {
        cout << "Error: 'valuesize' cannot be bigger than " << MAX_KVTSIZE << endl;
        exit(EXIT_FAILURE);
    }
    if (nflag && iflag) {
        cout << "Error: you cannot set both 'insertbytes' and 'numkeystoinsert' parameters" << endl;
        exit(EXIT_FAILURE);
    }
    if (sflag) {
        if (kflag) {
            cout << "Ignoring '-k' flag (keysize): keys will be read from stdin" << endl;
            kflag = 0;
            keysize = DEFAULT_KEY_SIZE;
        }
        if (vflag) {
            cout << "Ignoring '-v' flag (valuesize): values will be read from stdin" << endl;
            vflag = 0;
            valuesize = DEFAULT_VALUE_SIZE;
        }
        if (uflag) {
            cout << "Ignoring '-u' flag (unique keys): keys will be read from stdin" << endl;
            uflag = 0;
            unique_keys = DEFAULT_UNIQUE_KEYS;
        }
        if (zflag) {
            cout << "Ignoring '-z' flag (zipf keys): keys will be read from stdin" << endl;
            zflag = 0;
            zipf_keys = DEFAULT_ZIPF_KEYS;
        }
    }

    //--------------------------------------------------------------------------
    // create keyvalue store and set parameter values
    //--------------------------------------------------------------------------
    // Null compaction manager
    if (strcmp(compmanager, "nomerge") == 0) {
        kvstore = new KeyValueStore(KeyValueStore::NOMERGE_CM);
    }
    // Immediate compaction manager
    else if (strcmp(compmanager, "immediate") == 0) {
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
    // Rangemerge compaction manager
    else if (strcmp(compmanager, "rangemerge") == 0) {
        kvstore = new KeyValueStore(KeyValueStore::RNGMERGE_CM);
        if (bflag) {
            ((RangemergeCompactionManager *)kvstore->get_compaction_manager())->set_blocksize(blocksize);
        }
        if (fflag) {
            ((RangemergeCompactionManager *)kvstore->get_compaction_manager())->set_flushmem(flushmemorysize);
        }
    }
    // Cassandra compaction manager
    else if (strcmp(compmanager, "cassandra") == 0) {
        kvstore = new KeyValueStore(KeyValueStore::CASSANDRA_CM);
        if (lflag) {
            ((CassandraCompactionManager *)kvstore->get_compaction_manager())->set_L(cass_l);
        }
    }
    // other compaction manager?!
    else {
        cout << "Error: unknown compaction manager (but we should not get here!)" << endl;
        exit(EXIT_FAILURE);
    }

    //--------------------------------------------------------------------------
    // print values of parameters
    //--------------------------------------------------------------------------
    cout << "# compaction_manager:  " << setw(15) << compmanager << endl;
    cout << "# memory_size:         " << setw(15) << b2mb(memorysize) << " MB " << (mflag == 0 ? "(default)" : "")  << endl;
    if (sflag) {
        cout << "# insert_bytes:        " << setw(15) << "?" << "    (keys and values will be read from stdin)" << endl;
        cout << "# key_size:            " << setw(15) << "?" << "    (keys and values will be read from stdin)" << endl;
        cout << "# value_size:          " << setw(15) << "?" << "    (keys and values will be read from stdin)" << endl;
        cout << "# keys_to_insert:      " << setw(15) << "?" << "    (keys and values will be read from stdin)" << endl;
        cout << "# unique_keys:         " << setw(15) << "?" << "    (keys and values will be read from stdin)" << endl;
        cout << "# zipf_keys:           " << setw(15) << "?" << "    (keys and values will be read from stdin)" << endl;
    } else {
        cout << "# insert_bytes:        " << setw(15) << b2mb(insertbytes) << " MB " << (iflag == 0 ? "(default)" : "") << endl;
        cout << "# key_size:            " << setw(15) << keysize << "    " << (kflag == 0 ? "(default)" : "") << endl;
        cout << "# value_size:          " << setw(15) << valuesize << "    " << (vflag == 0 ? "(default)" : "") << endl;
        cout << "# keys_to_insert:      " << setw(15) << num_keys_to_insert << endl;
        cout << "# unique_keys:         " << setw(15) << (unique_keys ? "true" : "false") << "    " << (uflag == 0 ? "(default)" : "") << endl;
        cout << "# zipf_keys:           " << setw(15) << (zipf_keys ? "true" : "false") << "    " << (zflag == 0 ? "(default)" : "") << endl;
    }
    cout << "# put_throughput:      " << setw(15) << put_thrput << "    " << (Pflag == 0 ? "(default)" : "") << endl;
    cout << "# num_get_threads:     " << setw(15) << num_get_threads << "    " << (gflag == 0 ? "(default)" : "") << endl;
    cout << "# get_throughput:      " << setw(15) << get_thrput << "    " << (Gflag == 0 ? "(default)" : "") << endl;
    if (strcmp(compmanager, "geometric") == 0) {
        if (pflag == 0) {
            cout << "# geometric_r:         " << setw(15) << geom_r << "    " << (rflag == 0 ? "(default)" : "") << endl;
        } else {
            cout << "# geometric_p:         " << setw(15) << geom_p << "    " << (pflag == 0 ? "(default, disabled)" : "") << endl;
        }
    } else if (strcmp(compmanager, "rangemerge") == 0) {
        if (blocksize == 0) {
            cout << "# rngmerge_block_size: " << setw(15) << "inf" << " MB\n" << endl;
        } else {
            cout << "# rngmerge_block_size: " << setw(15) << b2mb(blocksize) << " MB " << (bflag == 0 ? "(default)" : "") << endl;
        }
        cout << "# rngmerge_flushmem_size: " << setw(12) << b2mb(flushmemorysize) << " MB " << (fflag == 0 ? "(default)" : "") << endl;
    }
    if (strcmp(compmanager, "cassandra") == 0) {
        cout << "# cassandra_l:         " << setw(15) << cass_l << "    " << (lflag == 0 ? "(default)" : "") << endl;
    }
    cout << "# read_from_stdin:     " << setw(15) << (sflag ? "true" : "false") << endl;
    cout << "# debug_level:         " << setw(15) << DBGLVL << endl;
    fflush(stdout);
//    system("svn info | grep Revision | awk '{printf \"# svn_revision:   %20d\\n\", $2}'");
    print_stats_header();

    //--------------------------------------------------------------------------
    // initialize variables
    //--------------------------------------------------------------------------
    kvstore->set_memstore_maxsize(memorysize);
    key = (char *)malloc(MAX_KVTSIZE);
    end_key = (char *)malloc(MAX_KVTSIZE);
    value = (char *)malloc(MAX_KVTSIZE);

    //--------------------------------------------------------------------------
    // fill-in arguments of put thread and get threads
    //--------------------------------------------------------------------------
    targs = (struct thread_args *)malloc((1 + num_get_threads) * sizeof(thread_args));
    for (i = 0; i < 1 + num_get_threads; i++) {
        targs[i].tid = i;
        targs[i].uflag = uflag;
        targs[i].sflag = sflag;
        targs[i].zipf_keys = zipf_keys;
        targs[i].print_kv_and_continue = print_kv_and_continue;
        targs[i].num_keys_to_insert = num_keys_to_insert;
        targs[i].keysize = keysize,
        targs[i].valuesize = valuesize;
        targs[i].kvstore = kvstore;
        targs[i].put_thrput = put_thrput;
        targs[i].get_thrput = get_thrput;
    }

    //--------------------------------------------------------------------------
    // create put thread and get threads
    //--------------------------------------------------------------------------
    thread = (pthread_t *)malloc((1 + num_get_threads) * sizeof(pthread_t));

    // create get threads
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

    if (!print_kv_and_continue) { // if we did insert some kvs to kvstore

        // if we crete unique keys, assert num keys in store equals num keys inserted
        if (uflag) {
            assert(kvstore->get_num_mem_keys() + kvstore->get_num_disk_keys() == num_keys_to_insert);
        }

        // flush remaining memory tuples
        while (kvstore->get_mem_size()) {
            kvstore->flush_bytes();
        }

        if (uflag) {
            assert(kvstore->get_num_mem_keys() == 0);
            assert(kvstore->get_num_disk_keys() == num_keys_to_insert);
        }
    }

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
void *put_routine(void *args)
{
    struct thread_args *targs = (struct thread_args *)args;
    int      uflag = targs->uflag,
             sflag = targs->sflag,
             zipf_keys = targs->zipf_keys,
             print_kv_and_continue = targs->print_kv_and_continue;
    uint64_t num_keys_to_insert = targs->num_keys_to_insert;
    uint32_t keysize = targs->keysize,
             valuesize = targs->valuesize;
    KeyValueStore *kvstore = targs->kvstore;
    uint32_t kseed = targs->tid, // kseed = getpid() + time(NULL);
             vseed = kseed + 1;
    char    *key = NULL,
            *value = NULL;
    uint32_t keylen, valuelen;
    uint64_t bytes_inserted = 0;
    RequestThrottle throttler(targs->put_thrput);

    if (DBGLVL > 0) {
        cout << "# [DEBUG]   put thread started" << endl;
    }

    key = (char *)malloc(MAX_KVTSIZE);
    value = (char *)malloc(MAX_KVTSIZE);

    // if we read keys and values from stdin, set num_keys_to_insert to 'infinity'
    if (sflag) {
        num_keys_to_insert = -1; // ('num_keys_to_insert' is uint64_t)
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
            //----------------------------------------------------------------------
            // read key and value from stdin
            //----------------------------------------------------------------------
            if (scanf("%s %s", key, value) != 2) {
                break;
            }
        } else {
            //----------------------------------------------------------------------
            // create a random key and a random value
            //----------------------------------------------------------------------
            if (zipf_keys) {
                zipfstr_r(key, keysize, &kseed);
            } else {
                randstr_r(key, keysize, &kseed);
            }
            if (uflag) {
                sprintf(key, "%s.%Ld", key, i); // make unique
            }
            randstr_r(value, valuesize, &vseed);
        }

        //----------------------------------------------------------------------
        // just print <key, value> to stdout, do not insert into kvstore
        //----------------------------------------------------------------------
        if (print_kv_and_continue) {
            cout << key << " " << value << endl;
            continue;
        }

        //----------------------------------------------------------------------
        // insert <key, value> into store
        //----------------------------------------------------------------------
        keylen = strlen(key); // make more efficient, e.g. keylen = keysize + sizeof(str(i) + 1) (+1 of '.' in sprintf above)
        valuelen = strlen(value); // make more efficient
        kvstore->put(key, keylen, value, valuelen);
        bytes_inserted += keylen + valuelen;
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
void *get_routine(void *args)
{
    struct thread_args *targs = (struct thread_args *)args;
    int      uflag = targs->uflag,
             zipf_keys = targs->zipf_keys;
    uint32_t keysize = targs->keysize;
    uint32_t kseed = targs->tid; // kseed = getpid() + time(NULL);
    char     key[MAX_KVTSIZE];
    int      i = 0;
    Scanner *scanner = new Scanner(targs->kvstore);
    RequestThrottle throttler(targs->get_thrput);

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
            zipfstr_r(key, keysize, &kseed);
        } else {
            randstr_r(key, keysize, &kseed);
        }
        if (uflag) {
            sprintf(key, "%s#%d", key, i++); // make unique
        }

        //--------------------------------------------------------------
        // execute range get()
        //--------------------------------------------------------------
        scanner->point_get(key);
//         strcpy(end_key, key);  // NOTE need a better way to create end key than this _bad_ hack! using this hack,
//         end_key[3] = 'z';      // NOTE as index grows, more and more keys fall within the range [key, end_key)
//         scanner->range_get(key, end_key);
    }

    if (DBGLVL > 0) {
        cout << "# [DEBUG]   get thread " << targs->tid << " finished" << endl;
    }

    delete scanner;
    pthread_exit(NULL);
}


/*============================================================================
 *                              randstr_r
 *============================================================================*/
void randstr_r(char *s, const int len, uint32_t *seed)
{
    static const char alphanum[] =
//         "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    int size = sizeof(alphanum);

    for (int i = 0; i < len; i++) {
        s[i] = alphanum[rand_r(seed) % (size - 1)];
    }

    s[len] = '\0';
}

/*============================================================================
 *                              zipfstr_r
 *============================================================================*/
void zipfstr_r(char *s, const int len, uint32_t *seed)
{
    int num_digits = log10(zipf_n) + 1;
    int zipf_num;
    char key_prefix[MAX_KVTSIZE];
    static bool first = true;
    uint32_t kseed = 0; // kseed = getpid() + time(NULL);

    if (first) {
        first = false;
        // key prefix must be common for all keys, so they follow zipf distribution
        randstr_r(key_prefix, len - num_digits, &kseed);
    }

    zipf_num = zipf_r(seed);
    sprintf(s, "%s%0*d", key_prefix, num_digits, zipf_num);
}

/*
 * code below from (has been modified for fastest number generation):
 *    http://www.csee.usf.edu/~christen/tools/toolpage.html
 * code was modified to precomput sum of probabilities and use integers
 * instead of doubles
 */

/*============================================================================
 *                               zipf_r
 *============================================================================*/
int zipf_r(uint32_t *seed)
{
    static int *sum_prob = NULL; // sum of probabilities
    int z,                       // uniform random number (0 <= z <= RAND_MAX)
        zipf_value,              // computed exponential value to be returned
        i,
        first, last, mid;        // for binary search

    // compute sum of probabilities on first call only
    if (sum_prob == NULL) {
        double *sum_prob_f;
        double c = 0;            // normalization constant

        for (i = 1; i <= zipf_n; i++) {
            c = c + (1.0 / pow((double) i, zipf_alpha));
        }
        c = 1.0 / c;

        // precompute sum of probabilities
        sum_prob_f = (double *)malloc((zipf_n + 1) * sizeof(double));
        sum_prob_f[0] = 0;
        for (i = 1; i <= zipf_n; i++) {
            sum_prob_f[i] = sum_prob_f[i-1] + c / pow((double) i, zipf_alpha);
        }

        // from array of doubles sum_prob_f[] that contains values in range
        // [0,1], compute array of integers sum_prob_i[] that contains values
        // in range [0,RAND_MAX]
        sum_prob = (int *)malloc((zipf_n + 1) * sizeof(int));
        for (i = 0; i <= zipf_n; i++) {
            sum_prob[i] = (int)(sum_prob_f[i] * RAND_MAX);
        }
    }

    // pull a uniform random number (0 <= z <= RAND_MAX)
    z = rand_r(seed);

    // map z to the value (find the first 'i' for which sum_prob[i] >= z)
    first = 1;
    last = zipf_n;
    while (first <= last) {
        mid = (last - first)/2 + first; // avoid overflow
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
    assert((zipf_value >= 1) && (zipf_value <= zipf_n));

    return(zipf_value);
}
