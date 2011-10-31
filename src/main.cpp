#include "Global.h"
#include "KeyValueStore.h"
#include "ImmCompactionManager.h"
#include "GeomCompactionManager.h"
#include "LogCompactionManager.h"
#include "RangemergeCompactionManager.h"
#include "Statistics.h"
#include "RangeScanner.h"

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
#include <pthread.h>

using namespace std;

#define CHECK_DUPLICATE_ARG(flag,opt) do {if (flag) { printf("Error: you have already set '-%c' argument\n", opt); exit(EXIT_FAILURE); }} while(0)

//------------------------------------------------------------------------------
// forward declaration of functions
//------------------------------------------------------------------------------

void  *put_routine(void *args);
void  *get_routine(void *args);
void   randstr(char *s, const int len);
void   zipfstr(char *s, const int len);
int    zipf();
double rand_val(int seed);

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

double zipf_alpha = 0.9;       // parameter for zipf() function
long int zipf_n = 1000000;     // parameter for zipf() function
// 'zipf_n' affects the generation time of a random zipf number: there
// is a loop in zipf() that goes from 1 to 'n'. So, the greater 'n' is,
// the more time zipf() needs to generate a zipf number.

struct put_args { // arguments for put thread
    int uflag;
    int sflag;
    int zipf_keys;
    int print_kv_and_continue;
    uint64_t num_keys_to_insert;
    uint32_t keysize;
    uint32_t valuesize;
    KeyValueStore *kvstore;
};

struct get_args { // arguments for get threads
    int uflag;
    int zipf_keys;
    uint32_t keysize;
    KeyValueStore *kvstore;
    int tid;
};

bool put_thread_finished = false;

/*============================================================================
 *                             print_syntax
 *============================================================================*/
void print_syntax(char *progname)
{
     printf("syntax: %s -c compactionmanager [options]\n", progname);
     printf("        -c compactionmanager:   \"nomerge\", \"immediate\", \"geometric\", \"logarithmic\", \"rangemerge\"\n");
     printf("        -r value:               r parameter, for geometric comp. manager only (default: %d)\n", DEFAULT_GEOM_R);
     printf("        -p value:               p parameter, for geometric comp. manager only (default: disabled)\n");
     printf("        -b blocksize:           block size in MB, for rangemerge comp. manager only (default: %.0f)\n", b2mb(DEFAULT_RNGMERGE_BLOCKSIZE));
     printf("        -f flushmem:            flush memory size in MB, for rangemerge comp. manager only (default: 0,\n");
     printf("                                when memory is full flush only the biggest range)\n");
     printf("        -i insertbytes:         number of bytes to insert in MB (default: %.0f)\n", b2mb(DEFAULT_INSERTBYTES));
     printf("        -n numkeystoinsert:     number of keys to insert (default: %Ld)\n", DEFAULT_INSERTKEYS);
     printf("        -k keysize:             size of keys, in bytes (default: %u)\n", DEFAULT_KEY_SIZE);
     printf("        -v valuesize:           size of values, in bytes (default: %u)\n", DEFAULT_VALUE_SIZE);
     printf("        -u:                     create unique keys (default: %s)\n", (DEFAULT_UNIQUE_KEYS) ? "true " : "false");
     printf("        -z:                     create zipfian keys (default: uniform keys)\n");
     printf("        -m memorysize:          memory size in MB (default: %.0f)\n", b2mb(DEFAULT_MEMSTORE_SIZE));
     printf("        -g numgetthreads:       number of get threads (default: %d)\n", DEFAULT_NUM_GET_THREADS);
     printf("        -o statsperiod:         every time that many MB are inserted, print stats by far.\n");
     printf("                                also, executed a number of gets() and print related stats\n");
     printf("                                (default: memorysize/2)\n");
     printf("        -s:                     read key-values from stdin\n");
     printf("        -e:                     print key-values that would be inserted and exit\n");
     printf("        -h:                     print this help message and exit\n");
}

/*============================================================================
 *                                main
 *============================================================================*/
int main(int argc, char **argv)
{
    char    *allargs = "hc:r:p:b:f:m:i:n:k:v:uzg:o:se";
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
             zflag = 0,
             gflag = 0,
             oflag = 0,
             sflag = 0,
             eflag = 0,
             myopt,
             i,
             geom_r,
             geom_p,
             num_get_threads,
             retval;
    uint64_t blocksize,
             flushmemorysize,
             memorysize,
             insertbytes,
             periodic_stats_step,
             num_keys_to_insert;
    uint32_t keysize,
             valuesize;
    char    *compmanager = NULL,
            *key = NULL,
            *value = NULL,
            *end_key = NULL,
            *ptr;
    bool     unique_keys,
             zipf_keys,
             print_kv_and_continue = false;
    KeyValueStore *kvstore;
    struct put_args pargs;
    struct get_args *gargs;
    pthread_t *thread;

    // we need at least compaction manager
    if (argc == 1) {
         print_syntax(argv[0]);
         exit(EXIT_FAILURE);
    }

    //--------------------------------------------------------------------------
    // get arguments
    //--------------------------------------------------------------------------
    while ((myopt = getopt (argc, argv, allargs)) != -1) {
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

        case 'z':
            CHECK_DUPLICATE_ARG(zflag, myopt);
            zflag = 1;
            zipf_keys = true;
            break;

        case 'g':
            CHECK_DUPLICATE_ARG(gflag, myopt);
            gflag = 1;
            num_get_threads = atoi(optarg);
            break;

        case 'o':
            CHECK_DUPLICATE_ARG(oflag, myopt);
            oflag = 1;
            periodic_stats_step = mb2b(atof(optarg));
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
            if ((ptr = strchr(allargs, optopt)) && *(ptr+1) == ':') {
                fprintf (stderr, "Error: option '-%c' requires an argument.\n", optopt);
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
    if (oflag == 0) {
        periodic_stats_step = memorysize/2;
    }

    //--------------------------------------------------------------------------
    // check values
    //--------------------------------------------------------------------------
    if (!cflag) {
        printf("Error: you must set compaction manager\n");
        exit(EXIT_FAILURE);
    }
    if (cflag && strcmp(compmanager, "nomerge") && strcmp(compmanager, "immediate") != 0
         && strcmp(compmanager, "geometric") != 0 && strcmp(compmanager, "logarithmic") != 0
         && strcmp(compmanager, "rangemerge") != 0) {
        printf("Error: compaction manager can be \"nomerge\", \"immediate\", \"geometric\", \"logarithmic\" or \"rangemerge\"\n");
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
    if (sflag) {
        if (kflag) {
            printf("Ignoring '-k' flag (keysize): keys will be read from stdin\n");
            kflag = 0;
            keysize = DEFAULT_KEY_SIZE;
        }
        if (vflag) {
            printf("Ignoring '-v' flag (valuesize): values will be read from stdin\n");
            vflag = 0;
            valuesize = DEFAULT_VALUE_SIZE;
        }
        if (uflag) {
            printf("Ignoring '-u' flag (unique keys): keys will be read from stdin\n");
            uflag = 0;
            unique_keys = DEFAULT_UNIQUE_KEYS;
        }
        if (zflag) {
            printf("Ignoring '-z' flag (zipf keys): keys will be read from stdin\n");
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
    // other compaction manager?!
    else {
        printf("Error: unknown compaction manager (but we should not get here!)\n");
        exit(EXIT_FAILURE);
    }

    //--------------------------------------------------------------------------
    // print values of parameters
    //--------------------------------------------------------------------------
    printf("# compaction_manager:  %15s\n", compmanager);
    printf("# memory_size:         %15.0f MB %s\n", b2mb(memorysize), (mflag == 0) ? "(default)" : "");
    if (sflag) {
        printf("# insert_bytes:        %15s    (keys and values will be read from stdin)\n", "?");
        printf("# key_size:            %15s    (keys and values will be read from stdin)\n", "?");
        printf("# value_size:          %15s    (keys and values will be read from stdin)\n", "?");
        printf("# keys_to_insert:      %15s    (keys and values will be read from stdin)\n", "?");
        printf("# unique_keys:         %15s    (keys and values will be read from stdin)\n", "?");
        printf("# zipf_keys:           %15s    (keys and values will be read from stdin)\n", "?");
    } else {
        printf("# insert_bytes:        %15.0f MB %s\n", b2mb(insertbytes), (iflag == 0) ? "(default)" : "");
        printf("# key_size:            %15u    %s\n", keysize, (kflag == 0) ? "(default)" : "");
        printf("# value_size:          %15u    %s\n", valuesize, (vflag == 0) ? "(default)" : "");
        printf("# keys_to_insert:      %15Ld\n", num_keys_to_insert);
        printf("# unique_keys:         %15s    %s\n", (unique_keys) ? "true" : "false", (uflag == 0) ? "(default)" : "");
        printf("# zipf_keys:           %15s    %s\n", (zipf_keys) ? "true" : "false", (zflag == 0) ? "(default)" : "");
    }
    printf("# num_get_threads:     %15d    %s\n", num_get_threads, (gflag == 0) ? "(default)" : "");
    printf("# print_stats_every:   %15.0f MB %s\n", b2mb(periodic_stats_step), (oflag == 0) ? "(default)" : "");
    if (strcmp(compmanager, "geometric") == 0) {
        if (pflag == 0) {
            printf("# geometric_r:         %15d    %s\n", geom_r, (rflag == 0) ? "(default)" : "");
        } else {
            printf("# geometric_p:         %15d    %s\n", geom_p, (pflag == 0) ? "(default, disabled)" : "");
        }
    } else if (strcmp(compmanager, "rangemerge") == 0) {
        if (blocksize == 0) {
            printf("# rngmerge_block_size: %15s MB\n", "inf");
        } else {
            printf("# rngmerge_block_size: %15.0f MB %s\n", b2mb(blocksize), (bflag == 0) ? "(default)" : "");
        }
        printf("# rngmerge_flushmem_size: %12.0f MB %s\n", b2mb(flushmemorysize), (fflag == 0) ? "(default)" : "");
    }
    printf("# memstore_merge_mode: %15s\n", (kvstore->get_memstore_merge_type() == CM_MERGE_ONLINE ? "online" : "offline"));
    printf("# read_from_stdin:     %15s\n", (sflag) ? "true" : "false");
    printf("# debug_level:         %15d\n", DBGLVL);
    fflush(stdout);
    system("svn info | grep Revision | awk '{printf \"# svn_revision:   %20d\\n\", $2}'");
    printf("# mb_ins | Ttotal Tcompac    Tput | Tmerge   Tfree Tcmrest |    Tmem   Tread  Twrite | mb_read  mb_writ    reads   writes | runs | avg_get  run_sizes\n");

    //--------------------------------------------------------------------------
    // initialize variables
    //--------------------------------------------------------------------------
    kvstore->set_memstore_maxsize(memorysize);
    key = (char *)malloc(MAX_KVTSIZE);
    end_key = (char *)malloc(MAX_KVTSIZE);
    value = (char *)malloc(MAX_KVTSIZE);
    rand_val(1.0); // must be called at least once with arg > 0, to seed Zipf randval()

    //--------------------------------------------------------------------------
    // fill-in arguments of put thread and get threads
    //--------------------------------------------------------------------------
    pargs.uflag = uflag;
    pargs.sflag = sflag;
    pargs.zipf_keys = zipf_keys;
    pargs.print_kv_and_continue = print_kv_and_continue;
    pargs.num_keys_to_insert = num_keys_to_insert;
    pargs.keysize = keysize,
    pargs.valuesize = valuesize;
    pargs.kvstore = kvstore;

    gargs = (struct get_args *)malloc(num_get_threads*sizeof(get_args));
    for (i = 0; i < num_get_threads; i++) {
        gargs[i].tid = i;
        gargs[i].uflag = uflag;
        gargs[i].zipf_keys = zipf_keys;
        gargs[i].keysize = keysize,
        gargs[i].kvstore = kvstore;
    }

    //--------------------------------------------------------------------------
    // create put thread and get threads
    //--------------------------------------------------------------------------
    thread = (pthread_t *)malloc((1 + num_get_threads)*sizeof(pthread_t));

    // create get threads
    for(i = 0; i < 1 + num_get_threads; i++) {
        if (i == 0) {
            retval = pthread_create(&thread[i], NULL, put_routine, (void *)&pargs);
        } else {
            retval = pthread_create(&thread[i], NULL, get_routine, (void *)&gargs[i-1]);
        }
        if (retval) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    //--------------------------------------------------------------------------
    // wait for threads to finish
    //--------------------------------------------------------------------------
    for(i = 0; i < 1 + num_get_threads; i++) {
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
    delete kvstore;

    return EXIT_SUCCESS;
}

/*============================================================================
 *                              put_routine
 *============================================================================*/
void *put_routine(void *args)
{
    struct put_args *pargs = (struct put_args *)args;
    int      uflag = pargs->uflag,
             sflag = pargs->sflag,
             zipf_keys = pargs->zipf_keys,
             print_kv_and_continue = pargs->print_kv_and_continue;
    uint64_t num_keys_to_insert = pargs->num_keys_to_insert;
    uint32_t keysize = pargs->keysize,
             valuesize = pargs->valuesize;
    KeyValueStore *kvstore = pargs->kvstore;
    char    *key = NULL,
            *value = NULL;
    uint64_t bytes_inserted = 0;

    printf("[DEBUG]  put thread started\n");

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
            if (zipf_keys) {    // TODO: randstr(), zipfstr should take as arg a random seed (See rand_r()).
                zipfstr(key, keysize);
            } else {
                randstr(key, keysize);
            }
            if (uflag) {
                sprintf(key, "%s.%Ld", key, i); // Make key unique by appending a unique number
            }
            randstr(value, valuesize);
        }

        //----------------------------------------------------------------------
        // just print <key, value> to stdout, do not insert into kvstore
        //----------------------------------------------------------------------
        if (print_kv_and_continue) {
            printf("%s %s\n", key, value);
            continue;
        }

        //----------------------------------------------------------------------
        // insert <key, value> into store
        //----------------------------------------------------------------------
        kvstore->put(key, value);
        bytes_inserted += strlen(key) + strlen(value);
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
    struct get_args *gargs = (struct get_args *)args;
    int      uflag = gargs->uflag,
             zipf_keys = gargs->zipf_keys;
    uint32_t keysize = gargs->keysize;
    unsigned int rseed;
    char     key[MAX_KVTSIZE];
    int i = 0;
    RangeScanner *scanner = new RangeScanner(gargs->kvstore);

    printf("[DEBUG]  thread %d started\n", gargs->tid);
    rseed = getpid();

    while (!put_thread_finished) {

        usleep(rand_r(&rseed) % 10000); // sleep for a random number of ms between 0 and 500000

        //--------------------------------------------------------------
        // create a random key
        //--------------------------------------------------------------
        if (zipf_keys) {        // TODO: randstr(), zipfstr should take as arg a random seed (See rand_r()).
            zipfstr(key, keysize);
        } else {
            randstr(key, keysize);
        }

        if (uflag) {
            sprintf(key, "%s#%d", key, i++); // make key unique AND NON EXISTING!
        }

        //--------------------------------------------------------------
        // execute range get()
        //--------------------------------------------------------------
        scanner->point_get(key);
//         strcpy(end_key, key);  // NOTE need a better way to create end key than this _bad_ hack! using this hack,
//         end_key[3] = 'z';      // NOTE as index grows, more and more keys fall within the range [key, end_key)
//         scanner->range_get(key, end_key);
    }

    delete scanner;
    pthread_exit(NULL);
}


/*============================================================================
 *                               randstr
 *============================================================================*/
void randstr(char *s, const int len)
{
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

/*============================================================================
 *                               zipfstr
 *============================================================================*/
void zipfstr(char *s, const int len)
{
    int num_digits = log10(zipf_n) + 1;
    int zipf_num;
    char key_prefix[MAX_KVTSIZE];
    static bool first = true;

    if (first) {
        first = false;
        // key prefix must be common for all keys, so they follow zipf distribution
        randstr(key_prefix, len - num_digits);
    }

    zipf_num = zipf();
    sprintf(s, "%s%0*d", key_prefix, num_digits, zipf_num);
}

/*
 * code below from (has been modified for fastest number generation):
 *    http://www.csee.usf.edu/~christen/tools/toolpage.html
 */

/*============================================================================
 *                                 zipf
 *============================================================================*/
int zipf()
{
    static bool first = true;     // Static first time flag
    static double c = 0;          // Normalization constant
    static double *sum_prob;      // Sum of probabilities
    double z;                     // Uniform random number (0 < z < 1)
    double zipf_value;            // Computed exponential value to be returned
    int    i;                     // Loop counter

    // Compute normalization constant on first call only
    if (first == true) {
        first = false;

        for (i = 1; i <= zipf_n; i++) {
            c = c + (1.0 / pow((double) i, zipf_alpha));
        }
        c = 1.0 / c;

        // gmargari mod: precompute sum of probabilities
        sum_prob = (double *)malloc((zipf_n + 1) * sizeof(double));
        sum_prob[1] = c / pow((double) 1, zipf_alpha);
        for (i = 2; i <= zipf_n; i++) {
            sum_prob[i] = sum_prob[i-1] + c / pow((double) i, zipf_alpha);
        }
    }

    // Pull a uniform random number (0 < z < 1)
    do {
        z = rand_val(0);
    } while ((z == 0) || (z == 1));

    // Map z to the value
    for (i = 1; i <= zipf_n; i++) {
        if (sum_prob[i] >= z) {
            zipf_value = i;
            break;
        }
    }

    // Assert that zipf_value is between 1 and N
    assert((zipf_value >= 1) && (zipf_value <= zipf_n));

    return(zipf_value);
}

/*============================================================================
 *                               rand_val
 *============================================================================*/
double rand_val(int seed)        // Return a random value between 0.0 and 1.0
{
    const long  a =      16807;  // Multiplier
    const long  m = 2147483647;  // Modulus
    const long  q =     127773;  // m div a
    const long  r =       2836;  // m mod a
    static long x;               // Random int value
    long        x_div_q;         // x divided by q
    long        x_mod_q;         // x modulo q
    long        x_new;           // New x value

    // Set the seed if argument is non-zero and then return zero
    if (seed > 0) {
        x = seed;
        return(0.0);
    }

    // RNG using integer arithmetic
    x_div_q = x / q;
    x_mod_q = x % q;
    x_new = (a * x_mod_q) - (r * x_div_q);
    if (x_new > 0) {
        x = x_new;
    } else {
        x = x_new + m;
    }

    // Return a random value between 0.0 and 1.0
    return((double) x / m);
}
