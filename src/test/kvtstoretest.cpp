#include "../KeyValueStore.h"

#include <cstdio>
#include <cstdlib>
#include <cassert>

#define BUFSIZE 1000

void randstr(char *s, const int len) {
    static const char alphanum[] =
        "0123456789"
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
    char *key, *value;
    struct timeval tv;
    int num_keys, maxkeysize, maxvaluesize;

    gettimeofday(&tv, NULL);
// tv.tv_usec = 104845;
    printf("seed: %ld\n", tv.tv_usec);

    srand(tv.tv_usec);
    num_keys = rand() % 50000 + 2000;
// num_keys = 3;
    maxkeysize = 10;
    maxvaluesize = 1000;
// maxkeysize = 3;
// maxvaluesize = 3;
    printf("num keys to be inserted: %d\n", num_keys);

    key = (char *)malloc(MAX_KVTSIZE);
    value = (char *)malloc(MAX_KVTSIZE);

    for (int i = 0; i < num_keys; i++) {
        randstr(key, (int)(rand() % maxkeysize) + 1);
        randstr(value, (int)(rand() % maxvaluesize) + 1);
        kvstore.put(key, value);
    }
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
