#include "../KeyValueStore.h"

#include <cstdio>
#include <cstdlib>
#include <cassert>

#define BUFSIZE 1000

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

int main(void)
{
    KeyValueStore kvstore;
    char *key, *value, *value2;
    struct timeval tv;
    int num_keys, maxkeysize, maxvaluesize;
    uint64_t timestamp;

    gettimeofday(&tv, NULL);
// tv.tv_usec = 923306;
    printf("seed: %ld\n", tv.tv_usec);

    srand(tv.tv_usec);
    num_keys = rand() % 30000 + 1000;
    maxkeysize = 20;
    maxvaluesize = 20;
// num_keys = 2000;
// maxkeysize = 10;
// maxvaluesize = 10;
    kvstore.set_memstore_maxsize(50000);
    printf("memstore size: %Ld\n", kvstore.get_memstore_maxsize());
    printf("num keys to be inserted: %d\n", num_keys);
    printf("maxkeysize: %d\n", maxkeysize);
    printf("maxvaluesize: %d\n", maxvaluesize);

    key = (char *)malloc(MAX_KVTSIZE);
    value = (char *)malloc(MAX_KVTSIZE);

    //================================================================
    // insert values
    //================================================================
    printf("Insert key-values\n");
    for (int i = 0; i < num_keys; i++) {
        randstr(key, (int)(rand() % maxkeysize) + 1);
        randstr(value, (int)(rand() % maxvaluesize) + 1);
        sprintf(key, "%s%d", key, i); // create unique keys, so no values are overwritten
        kvstore.put(key, value);
        if (i % 100 == 0)
            dbg_i(i);
    }

    printf("\nSearch all keys\n");

    //================================================================
    // search values (most of them should be on disk, latest should
    // be on memory)
    //================================================================
    srand(tv.tv_usec);
    rand(); // to get the same sequence as above

    for (int i = 0; i < num_keys; i++) {
        randstr(key, (int)(rand() % maxkeysize) + 1);
        randstr(value, (int)(rand() % maxvaluesize) + 1);
        sprintf(key, "%s%d", key, i);
        if (kvstore.get(key, &value2, &timestamp) == false) {
            printf("Key [%s] was not found!\n", key);
            return EXIT_FAILURE;
        }
        assert(strcmp(value, value2) == 0);
        free(const_cast<char*>(value2)); // since value returned from get is always a copy...
        if (i % 100 == 0)
            dbg_i(i);
    }

    assert(kvstore.num_mem_keys() + kvstore.num_disk_keys() == num_keys);  // since keys are unique
    printf("Memstore keys:  %Ld\n", kvstore.num_mem_keys());
    printf("Diskstore keys: %Ld\n", kvstore.num_disk_keys());
    printf("Memstore size:  %Ld\n", kvstore.mem_size());
    printf("Diskstore size: %Ld\n", kvstore.disk_size());

    free(key);
    free(value);

//     size_t numkvts = 13;
//     char *strs[13][2] = {
//         {(char *)"d",      (char *)"d1"},
//         {(char *)"aaa",    (char *)"aa1"},
//         {(char *)"c",      (char *)"c0"},
//         {(char *)"a",      (char *)"a1"},
//         {(char *)"x",      (char *)"x1"},
//         {(char *)"c",      (char *)"c1"},
//         {(char *)"f",      (char *)"f2"},
//         {(char *)"q",      (char *)"q2"},
//         {(char *)" ",      (char *)"space"},
//         {(char *)"space",  (char *)" "},
//         {(char *)"aaa",    (char *)"xxxxx1"},
//         {(char *)"c",      (char *)"c2"},
//         {(char *)"t",      (char *)"t2"},
//     };
//
//     for (size_t i = 0; i < numkvts - 5; i++) {
//         kvstore.put(strs[i][0], strs[i][1]);
//     }
//
//     for (size_t i = 0; i < numkvts - 3; i++) {
//         kvstore.put(strs[i][0], strs[i][1]);
//     }
//
//     for (size_t i = 0; i < numkvts; i++) {
//         kvstore.put(strs[i][0], strs[i][1]);
//     }


//     kvstore.put("a",  "a1");
//     kvstore.put("b",  "b2");
//     kvstore.put("c",  "c3");
//     kvstore.put("d",  "d4");
//     kvstore.put("e",  "e5");
//     kvstore.put("f",  "f6");
//     kvstore.put("g",  "g7");
//     kvstore.put("h",  "h8");
//     kvstore.put("j",  "j9");
//     kvstore.put("k",  "k10");
//     kvstore.put("a",  "a11");
//     kvstore.put("aa", "aa12");
//     kvstore.put("b",  "b13");
//     kvstore.put("c",  "c14");
//     kvstore.put("a",  "a15");
//     kvstore.put("c",  "c16");
//     kvstore.put("d",  "d17");
//     kvstore.put("b",  "b18");
//     kvstore.put("a",  "a1x");
//     kvstore.put("b",  "b2x");
//     kvstore.put("c",  "c3x");
//     kvstore.put("d",  "d4x");
//     kvstore.put("e",  "e5x");
//     kvstore.put("f",  "f6x");
//     kvstore.put("g",  "g7x");
//     kvstore.put("h",  "h8x");
//     kvstore.put("j",  "j9x");
//     kvstore.put("k",  "k10x");
//     kvstore.put("a",  "a11x");
//     kvstore.put("aa", "aa12x");
//     kvstore.put("b",  "b13x");
//     kvstore.put("c",  "c14x");
//     kvstore.put("a",  "a15x");
//     kvstore.put("c",  "c16x");
//     kvstore.put("d",  "d17x");
//     kvstore.put("b",  "b18x");
//     kvstore.put("lastkey",  "lalala");

//     assert(kvstore.num_mem_keys() == numkvts - 3); // 3 duplicate keys
//     assert(strcmp(kvstore.get((char *)"d"),     (char *)"d1") == 0);
//     assert(strcmp(kvstore.get((char *)"aaa"),   (char *)"aa1") != 0);    // should have been replaced
//     assert(strcmp(kvstore.get((char *)"aaa"),   (char *)"xxxxx1") == 0);
//     assert(strcmp(kvstore.get((char *)"c"),     (char *)"c1") != 0);     // should have been replaced
//     assert(strcmp(kvstore.get((char *)"c"),     (char *)"c2") == 0);
//     assert(strcmp(kvstore.get((char *)"a"),     (char *)"a1") == 0);
//     assert(strcmp(kvstore.get((char *)"x"),     (char *)"x1") == 0);
//     assert(strcmp(kvstore.get((char *)" "),     (char *)"space") == 0);
//     assert(strcmp(kvstore.get((char *)"space"), (char *)" ") == 0);
//     assert(kvstore.get((char *)"nonexisting") == NULL);
//     assert(kvstore.get((char *)"/") == NULL);
//     assert(kvstore.get((char *)"\xf3") == NULL);
//     assert(kvstore.num_mem_keys() == numkvts - 3); // 3 duplicate keys
//

    printf("Everything ok!\n");

    return EXIT_SUCCESS;
}
