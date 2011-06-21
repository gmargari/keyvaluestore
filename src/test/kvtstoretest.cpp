#include "../Global.h"
#include "../KeyValueStore.h"

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <sys/time.h>

#define MEMSTORE_SIZE        1000000LL
#define BYTES_TO_INSERT     10000000LL
#define MAX_KEY_SIZE               100
#define MAX_VALUE_SIZE            1000
#define SEARCH_SKIP_KEYS           100  // search 1 every 'SEARCH_SKIP_KEYS' keys
#define UNIQUE_KEYS                  1  // create unique keys, so no values are overwritten

#define BUFSIZE 1000

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
int main(void)
{
    KeyValueStore *kvstore;
    char *key, *value, *value2;
    struct timeval tv;
    uint64_t timestamp, num_keys, i;

    kvstore = new KeyValueStore(KeyValueStore::GEOM_CM);
    kvstore->set_memstore_maxsize(MEMSTORE_SIZE);

    gettimeofday(&tv, NULL);
// tv.tv_usec = 118817;
    printf("seed: %ld\n", tv.tv_usec);

    srand(tv.tv_usec);
    num_keys = rand() % 100000;
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
            sprintf(key, "%s%Ld", key, i);
        }
        kvstore->put(key, value);
        if (i && i % 100 == 0)
            printf("i = %Ld, memstore size: %Ld\n", i, kvstore->get_mem_size());
    }

    //================================================================
    // search values (most of them should be on disk, latest should
    // be on memory)
    //================================================================
    printf("\n===== Search all keys =====\n");
    srand(tv.tv_usec);

    for (i = 0; i < num_keys; i++) {
//         randstr(key, (int)(rand() % MAX_KEY_SIZE) + 1);
//         randstr(value, (int)(rand() % MAX_VALUE_SIZE) + 1);
        randstr(key, MAX_KEY_SIZE);
        randstr(value, MAX_VALUE_SIZE);

        // search one every 'SEARCH_SKIP_KEYS' keys
        if (i && i % SEARCH_SKIP_KEYS == 0)
            continue;

        if (UNIQUE_KEYS) {
            sprintf(key, "%s%Ld", key, i);
        }
        if (kvstore->get(key, &value2, &timestamp) == false) {
            printf("Key [%s] was not found!\n", key);
            return EXIT_FAILURE;
        }
        if (UNIQUE_KEYS) {
            assert(strcmp(value, value2) == 0);
        }
        free(const_cast<char*>(value2)); // since value returned from get is always a copy...
        if (i && i % 100 == 0)
            printf("i = %Ld\n", i);
    }

    printf("Memstore keys:  %Ld\n", kvstore->get_num_mem_keys());
    printf("Diskstore keys: %Ld\n", kvstore->get_num_disk_keys());
    printf("Memstore size:  %Ld\n", kvstore->get_mem_size());
    printf("Diskstore size: %Ld\n", kvstore->get_disk_size());

    if (UNIQUE_KEYS) {
        assert(kvstore->get_num_mem_keys() + kvstore->get_num_disk_keys() == num_keys);
    }

    free(key);
    free(value);
    delete kvstore;

    printf("Everything ok!\n");

    return EXIT_SUCCESS;
}
