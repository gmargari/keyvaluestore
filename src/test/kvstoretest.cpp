#include "../Global.h"
#include "../KeyValueStore.h"
#include "../RangeScanner.h"

#include <cstdio>
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
    RangeScanner *scanner;

    kvstore = new KeyValueStore(KeyValueStore::GEOM_CM);
    kvstore->set_memstore_maxsize(MEMSTORE_SIZE);
    scanner = new RangeScanner(kvstore);

    gettimeofday(&tv, NULL);
// tv.tv_usec = 118817;
    printf("seed: %ld\n", tv.tv_usec);

    srand(tv.tv_usec);
//     num_keys = rand() % 100000;
    num_keys = BYTES_TO_INSERT / (MAX_KEY_SIZE + MAX_VALUE_SIZE);

    printf("memstore size:   %12Ld\n", MEMSTORE_SIZE);
    printf("bytes to insert: %12Ld\n", BYTES_TO_INSERT);
    printf("keys to insert:  %12Ld\n", num_keys);
    printf("max key size:    %12d\n", MAX_KEY_SIZE);
    printf("max value size:  %12d\n", MAX_VALUE_SIZE);

    key = (char *)malloc(MAX_KVTSIZE);
    value = (char *)malloc(MAX_KVTSIZE);

    //================================================================
    // insert values
    //================================================================

    printf("===== Insert key-values =====\n");
    srand(tv.tv_usec);

    for (i = 0; i < num_keys; i++) {
//         randstr(key, (int)(rand() % MAX_KEY_SIZE) + 1);
//         randstr(value, (int)(rand() % MAX_VALUE_SIZE) + 1);
        randstr(key, MAX_KEY_SIZE);
        randstr(value, MAX_VALUE_SIZE);
        if (UNIQUE_KEYS) {
            sprintf(key + strlen(key), "%Ld", i);
        }
        kvstore->put(key, value);
        if (i && i % 100 == 0)
            printf("i = %Ld, memstore size: %Ld\n", i, kvstore->get_mem_size());
    }

    //================================================================
    // search values (most of them should be on disk, latest should
    // be on memory)
    //================================================================
    printf("\n===== Search every %dth key =====\n", SEARCH_SKIP_KEYS);
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
        if (!scanner->point_get(key)) {
            printf("%d) Key [%s] was not found!\n", i, key);
            exit(EXIT_FAILURE);
        }
    }

    printf("Memstore keys:  %Ld\n", kvstore->get_num_mem_keys());
    printf("Diskstore keys: %Ld\n", kvstore->get_num_disk_keys());
    printf("Memstore size:  %Ld\n", kvstore->get_mem_size());
    printf("Diskstore size: %Ld\n", kvstore->get_disk_size());

    if (UNIQUE_KEYS) {
        if (kvstore->get_num_mem_keys() + kvstore->get_num_disk_keys() != num_keys) {
            printf("mem keys (%Ld) + disk keys (%Ld) != keys inserted (%Ld)\n",
              kvstore->get_num_mem_keys(), kvstore->get_num_disk_keys(), num_keys);
            exit(EXIT_FAILURE);
        }
    }

    free(key);
    free(value);
    delete kvstore;
    delete scanner;

    printf("Everything ok!\n");

    return EXIT_SUCCESS;
}
