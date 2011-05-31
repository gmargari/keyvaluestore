#include <stdio.h>
#include <stdlib.h>

#include "KeyValueStore.h"

#include <cassert>

#define BUFSIZE 1000

int main(void)
{
    KeyValueStore kvstore;
    size_t numkvs = 10;
    char *strs[10][2] = {
        {(char *)"d",      (char *)"d1"},
        {(char *)"aaa",    (char *)"aa1"},
        {(char *)"c",      (char *)"c0"},
        {(char *)"a",      (char *)"a1"},
        {(char *)"x",      (char *)"x1"},
        {(char *)"c",      (char *)"c1"},
        {(char *)" ",      (char *)"space"},
        {(char *)"space",  (char *)" "},
        {(char *)"aaa",    (char *)"xxxxx1"},
        {(char *)"c",      (char *)"c2"},       
    };

    for (size_t i = 0; i < numkvs; i++) {
        kvstore.put(strs[i][0], strs[i][1]);
    }

    assert(kvstore.num_mem_keys() == numkvs - 3); // 3 duplicate keys
    assert(strcmp(kvstore.get((char *)"d"),     (char *)"d1") == 0);
    assert(strcmp(kvstore.get((char *)"aaa"),   (char *)"aa1") != 0);    // should have been replaced
    assert(strcmp(kvstore.get((char *)"aaa"),   (char *)"xxxxx1") == 0);
    assert(strcmp(kvstore.get((char *)"c"),     (char *)"c1") != 0);     // should have been replaced
    assert(strcmp(kvstore.get((char *)"c"),     (char *)"c2") == 0);
    assert(strcmp(kvstore.get((char *)"a"),     (char *)"a1") == 0);
    assert(strcmp(kvstore.get((char *)"x"),     (char *)"x1") == 0);
    assert(strcmp(kvstore.get((char *)" "),     (char *)"space") == 0);
    assert(strcmp(kvstore.get((char *)"space"), (char *)" ") == 0);
    assert(kvstore.get((char *)"nonexisting") == NULL);
    assert(kvstore.get((char *)"/") == NULL);
    assert(kvstore.get((char *)"\xf3") == NULL);
    assert(kvstore.num_mem_keys() == numkvs - 3); // 3 duplicate keys

    kvstore.dumpmem();
    
    printf("Everything ok!\n");
}

