#include "../Global.h"
#include "../KeyValueStore.h"
#include "../Scanner.h"

#include <cstdio>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <sys/time.h>

#define MEMSTORE_SIZE        1000000LL
#define BYTES_TO_INSERT     10000000LL
#define MAX_KEY_SIZE               100
#define MAX_VALUE_SIZE            1000
#define SEARCH_SKIP_KEYS             1  // search 1 every 'SEARCH_SKIP_KEYS' keys
#define UNIQUE_KEYS                  1  // create unique keys, so no values are overwritten

#define BUFSIZE 1000

using namespace std;

/*============================================================================
 *                               randstr
 *============================================================================*/
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

/*============================================================================
 *                                main
 *============================================================================*/
int main()
{
    KeyValueStore *kvstore;
    char *key, *value;
    struct timeval tv;
    uint64_t num_keys, i;
    Scanner *scanner;

    kvstore = new KeyValueStore(KeyValueStore::GEOM_CM);
    kvstore->set_memstore_maxsize(MEMSTORE_SIZE);
    scanner = new Scanner(kvstore);

    gettimeofday(&tv, NULL);
// tv.tv_usec = 118817;
    cout << "seed: " << tv.tv_usec << endl;

    srand(tv.tv_usec);
//     num_keys = rand() % 100000;
    num_keys = BYTES_TO_INSERT / (MAX_KEY_SIZE + MAX_VALUE_SIZE);

    cout << "memstore size:   " << setw(12) << right << MEMSTORE_SIZE << endl;
    cout << "bytes to insert: " << setw(12) << right << BYTES_TO_INSERT << endl;
    cout << "keys to insert:  " << setw(12) << right << num_keys << endl;
    cout << "max key size:    " << setw(12) << right << MAX_KEY_SIZE << endl;
    cout << "max value size:  " << setw(12) << right << MAX_VALUE_SIZE << endl;

    key = (char *)malloc(MAX_KVTSIZE);
    value = (char *)malloc(MAX_KVTSIZE);

    //================================================================
    // insert values
    //================================================================

    cout << "===== Insert key-values =====" << endl;
    srand(tv.tv_usec);

    for (i = 0; i < num_keys; i++) {
//         randstr(key, (int)(rand() % MAX_KEY_SIZE) + 1);
//         randstr(value, (int)(rand() % MAX_VALUE_SIZE) + 1);
        randstr(key, MAX_KEY_SIZE);
        randstr(value, MAX_VALUE_SIZE);
        if (UNIQUE_KEYS) {
            sprintf(key + strlen(key), "%Ld", i);
        }
        kvstore->put(key, strlen(key), value, strlen(value));
        if (i && i % 100 == 0)
            cout << "i = " << i << ", memstore size = " << kvstore->get_mem_size() << endl;
    }

    // flush remaining memory tuples
    while (kvstore->get_mem_size()) {
        kvstore->flush_bytes();
    }

    //================================================================
    // search values (most of them should be on disk, latest should
    // be on memory)
    //================================================================
    cout << endl << "===== Search every " << SEARCH_SKIP_KEYS << "th key =====" << endl;
    srand(tv.tv_usec);

    for (i = 0; i < num_keys; i++) {
//         randstr(key, (int)(rand() % MAX_KEY_SIZE) + 1);
//         randstr(value, (int)(rand() % MAX_VALUE_SIZE) + 1);
        randstr(key, MAX_KEY_SIZE);
        randstr(value, MAX_VALUE_SIZE);

        // search one every 'SEARCH_SKIP_KEYS' keys
        if (SEARCH_SKIP_KEYS > 1 && i && i % SEARCH_SKIP_KEYS == 0)
            continue;

        if (UNIQUE_KEYS) {
            sprintf(key + strlen(key), "%Ld", i);
        }
        if (!scanner->point_get(key, strlen(key))) {
            cout << i << ") Key [" << key << "] was not found!" << endl;
            exit(EXIT_FAILURE);
        }
    }

    cout << "Memstore keys:  " << kvstore->get_num_mem_keys() << endl;
    cout << "Diskstore keys: " << kvstore->get_num_disk_keys() << endl;
    cout << "Memstore size:  " << kvstore->get_mem_size() << endl;
    cout << "Diskstore size: " << kvstore->get_disk_size() << endl;

    if (UNIQUE_KEYS) {
        if (kvstore->get_num_mem_keys() + kvstore->get_num_disk_keys() != num_keys) {
            cout << "(mem keys: " << kvstore->get_num_mem_keys() << ") + (disk keys: "
                 << kvstore->get_num_disk_keys() << ") != (keys inserted: " << num_keys << ")" << endl;
            exit(EXIT_FAILURE);
        }
    }

    free(key);
    free(value);
    delete kvstore;
    delete scanner;

    cout << "Everything ok!" << endl;

    return EXIT_SUCCESS;
}
