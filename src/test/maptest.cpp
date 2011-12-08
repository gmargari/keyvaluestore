#include "../Global.h"
#include "../MapInputStream.h"
#include "../DiskFile.h"
#include "../DiskFileInputStream.h"
#include "../DiskFileOutputStream.h"
#include "../PriorityInputStream.h"
#include "../Statistics.h"

#include <iostream>
#include <cstdlib>
#include <cassert>
#include <cstdlib>
#include <sys/time.h>

using namespace std;

#define BUFSIZE 1000

/*============================================================================
 *                               randstr
 *============================================================================*/
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

/*============================================================================
 *                                 main
 *============================================================================*/
int main()
{
    Map *map;
    MapInputStream *istream, *istream1, *istream2, *istream3, *istream4;
    PriorityInputStream *pistream;
    DiskFile *file1, *file2;
    DiskFileOutputStream *ostream1, *ostream2;
    DiskFileInputStream *dfistream1, *dfistream2;
    vector<InputStream *> istreams;
    char *key, *value, *key1, *key2, *key3, *value1, *prev_key;
    const char *k1, *v1, *k2, *v2;
    struct timeval tv;
    int num_keys, num, maxkeysize, maxvaluesize;
    uint64_t timestamp, ts1, ts2;

    gettimeofday(&tv, NULL);
// tv.tv_usec = 693157;
    cout << "seed: " << tv.tv_usec << endl;

    map = new Map();
    srand(tv.tv_usec);
    num_keys = rand() % 5000;
// num_keys = 10;
    maxkeysize = 10;
    maxvaluesize = 10;
// maxkeysize = 3;
// maxvaluesize = 3;
    cout << "num keys to be inserted: " << num_keys << endl;

    global_stats_init(); // avoid assertion error...

    //========================================================
    // insert values in memstore
    //========================================================

    srand(tv.tv_usec);

    key = (char *)malloc(MAX_KVTSIZE);
    value = (char *)malloc(MAX_KVTSIZE);
    prev_key = (char *)malloc(MAX_KVTSIZE);

    for (int i = 0; i < num_keys; i++) {
        size_t keylen = rand() % maxkeysize + 1,
               valuelen = rand() % maxkeysize + 1;
        randstr(key, keylen);
        randstr(value, valuelen);
        map->put(key, keylen, value, valuelen, i+1);
    }

    //========================================================
    // write all values to disk file
    //========================================================
    file1 = new DiskFile();
    file1->open_new_unique();
    istream = new MapInputStream(map);
    ostream1 = new DiskFileOutputStream(file1, MERGE_BUFSIZE);
    istream->set_key_range(NULL, NULL);
    prev_key[0] = '\0';
    while (istream->read(&k1, &v1, &timestamp)) {
        assert(strcmp(k1, prev_key) != 0);
        ostream1->append(k1, strlen(k1), v1, strlen(v1), timestamp);
        strcpy(prev_key, k1);
    }

    ostream1->close();
    map->clear();

    //========================================================
    // insert the same values to memstore
    //========================================================
    srand(tv.tv_usec);
    for (int i = 0; i < num_keys; i++) {
        size_t keylen = rand() % maxkeysize + 1,
               valuelen = rand() % maxkeysize + 1;
        randstr(key, keylen);
        randstr(value, valuelen);
        map->put(key, keylen, value, valuelen, i+1);
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

    istream1 = new MapInputStream(map);
    istream2 = new MapInputStream(map);
    istream3 = new MapInputStream(map);
    istream4 = new MapInputStream(map);

    istream1->set_key_range(NULL, key1);
    istream2->set_key_range(key1, key2);
    istream3->set_key_range(key2, key3);
    istream4->set_key_range(key3, NULL);
    istreams.push_back(istream1);
    istreams.push_back(istream2);
    istreams.push_back(istream3);
    istreams.push_back(istream4);

    //========================================================
    // merge the 4 streams create above, using priority stream,
    // and store results to a new file
    //========================================================
    file2 = new DiskFile();
    file2->open_new_unique();
    ostream2 = new DiskFileOutputStream(file2, MERGE_BUFSIZE);
    pistream = new PriorityInputStream(istreams);
    prev_key[0] = '\0';
    while (pistream->read(&k1, &v1, &timestamp)) {
        assert(strcmp(k1, prev_key) != 0);
        ostream2->append(k1, strlen(k1), v1, strlen(v1), timestamp);
        strcpy(prev_key, k1);
    }
    ostream2->close();

    map->clear();
    assert(map->get_size() == 0);
    assert(map->get_num_keys() == 0);

    //========================================================
    // check that the two files have identical contents
    //========================================================
    value1 = (char *)malloc(MAX_KVTSIZE);
    dfistream1 = new DiskFileInputStream(file1, MERGE_BUFSIZE);
    dfistream2 = new DiskFileInputStream(file2, MERGE_BUFSIZE);
    dfistream1->set_key_range(NULL, NULL);
    dfistream2->set_key_range(NULL, NULL);
    num = 0;
    while (dfistream1->read(&k1, &v1, &ts1)) {
        dfistream2->read(&k2, &v2, &ts2);
        if (strcmp(k1, k2) != 0) {
            cout << num << ") k1: " << k1 << " != k2: " << k2 << endl;
            exit(EXIT_FAILURE);
        }
        if (strcmp(v1, v2) != 0) {
            cout << num << ") v1: " << v1 << " != v2: " << v2 << endl;
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
    file1->delete_from_disk();
    file2->delete_from_disk();
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
    delete pistream;
    delete map;

    cout << "Everything ok!" << endl;

    return EXIT_SUCCESS;
}
