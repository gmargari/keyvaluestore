#include "../KVTMapInputStream.h"
#include "../KVTPriorityInputStream.h"
#include "../KVTDiskFileInputStream.h"
#include "../KVTDiskFileInputStream.h"
#include "../CompactionManager.h"

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <stdlib.h>

#define BUFSIZE 1000

void randstr(char *s, const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; i++) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len] = '\0';
}


int main(void)
{
    KVTMap *map;
    KVTMapInputStream *istream, *istream1, *istream2, *istream3, *istream4;
    KVTPriorityInputStream *istream_heap;
    KVTDiskFile *file1, *file2;
    KVTDiskFileOutputStream *ostream1, *ostream2;
    KVTDiskFileInputStream *dfistream1, *dfistream2;
    vector<KVTInputStream *> istreams;
    char *key, *value, *key1, *key2, *key3, *value1, *prev_key;
    const char *k1, *v1, *k2, *v2;
    struct timeval tv;
    int num_keys, num, maxkeysize, maxvaluesize;
    uint64_t timestamp, prev_timestamp, ts1, ts2;

    gettimeofday(&tv, NULL);
// tv.tv_usec = 438701;
    printf("seed: %ld\n", tv.tv_usec);

    map = new KVTMap();
    srand(tv.tv_usec);
    num_keys = rand() % 5000;
// num_keys = 10;
    maxkeysize = 10;
    maxvaluesize = 10;
// maxkeysize = 3;
// maxvaluesize = 3;
    printf("num keys to be inserted: %d\n", num_keys);

    //========================================================
    // insert values in memstore
    //========================================================

    srand(tv.tv_usec);

    key = (char *)malloc(MAX_KVTSIZE);
    value = (char *)malloc(MAX_KVTSIZE);
    prev_key = (char *)malloc(MAX_KVTSIZE);

    for (int i = 0; i < num_keys; i++) {
        randstr(key, (int)(rand() % maxkeysize) + 1);
        randstr(value, (int)(rand() % maxvaluesize) + 1);
        map->put(key, value);
    }

    //========================================================
    // write all values to disk file
    //========================================================
    file1 = new KVTDiskFile();
    file1->open_unique();
    istream = new KVTMapInputStream(map);
    ostream1 = new KVTDiskFileOutputStream(file1);
    prev_timestamp = 0;
    prev_key[0] = '\0';
    while (istream->read(&k1, &v1, &timestamp)) {
        ostream1->write(k1, v1, timestamp);
        if (strcmp(k1, prev_key) == 0) {
            assert(timestamp > prev_timestamp);
        }
        prev_timestamp = timestamp;
        strcpy(prev_key, k1);
    }

    ostream1->flush();
    map->clear();

    //========================================================
    // insert the same values to memstore
    //========================================================
    srand(tv.tv_usec);
    for (int i = 0; i < num_keys; i++) {
        randstr(key, (int)(rand() % maxkeysize) + 1);
        randstr(value, (int)(rand() % maxvaluesize) + 1);
        map->put(key, value);
    }

    //========================================================
    // create 4 distinct subranges of keys, create a stream for each, and
    // then create a priority stream with all 4 streams
    //========================================================
    key1 = (char *)malloc(MAX_KVTSIZE);
    key2 = (char *)malloc(MAX_KVTSIZE);
    key3 = (char *)malloc(MAX_KVTSIZE);
    strcpy(key1, "fff");
    strcpy(key2, "mmm");
    strcpy(key3, "ttt");
//     printf("ranges: [NULL, %s) [%s, %s) [%s, %s) [%s, NULL]\n", key1, key1, key2, key2, key3, key3);

    istream1 = new KVTMapInputStream(map, NULL, key1);
    istream2 = new KVTMapInputStream(map, key1, key2);
    istream3 = new KVTMapInputStream(map, key2, key3);
    istream4 = new KVTMapInputStream(map, key3, NULL);
    istreams.push_back(istream1);
    istreams.push_back(istream2);
    istreams.push_back(istream3);
    istreams.push_back(istream4);

    //========================================================
    // merge the 4 streams create above, using priority stream,
    // and store results to a new file
    //========================================================
    file2 = new KVTDiskFile();
    file2->open_unique();
    ostream2 = new KVTDiskFileOutputStream(file2);
    istream_heap = new KVTPriorityInputStream(istreams);
    prev_timestamp = 0;
    prev_key[0] = '\0';
    while (istream_heap->read(&k1, &v1, &timestamp)) {
        ostream2->write(k1, v1, timestamp);
        if (strcmp(k1, prev_key) == 0) {
            assert(timestamp > prev_timestamp);
        }
        prev_timestamp = timestamp;
        strcpy(prev_key, k1);
    }

    ostream2->flush();
    map->clear(NULL, key1);
    map->clear(key1, key2);
    map->clear(key2, key3);
    map->clear(key3, NULL);
    assert(map->size() == 0);
    assert(map->num_keys() == 0);

    //========================================================
    // check that the two files have identical contents
    //========================================================
    value1 = (char *)malloc(MAX_KVTSIZE);
    dfistream1 = new KVTDiskFileInputStream(file1);
    dfistream2 = new KVTDiskFileInputStream(file2);
    num = 0;
    while (dfistream1->read(&k1, &v1, &ts1)) {
        dfistream2->read(&k2, &v2, &ts2);
        if (strcmp(k1, k2) != 0) {
            printf("%d) k1: %s != k2: %s\n", num, k1, k2);
            exit(EXIT_FAILURE);
        }
        if (strcmp(v1, v2) != 0) {
            printf("%d) v1: %s != v2: %s\n", num, v1, v2);
            exit(EXIT_FAILURE);
        }
        num++;
    }
    assert(dfistream2->read(&k2, &v2, &ts2) == false);

    //========================================================
    // free mem / delete objects
    //========================================================
    free(key);
    free(value);
    free(key1);
    free(key2);
    free(key3);
    free(value1);
    free(prev_key);
//     file1->delete_from_disk();
//     file2->delete_from_disk();
    delete file1;
    delete file2;
    delete istream;
    delete istream1;
    delete istream2;
    delete istream3;
    delete istream4;
    delete ostream1;
    delete ostream2;
    delete dfistream1;
    delete dfistream2;
    delete istream_heap;
    delete map;

    printf("Everything ok!\n");

    return EXIT_SUCCESS;
}
