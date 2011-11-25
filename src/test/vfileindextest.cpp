#include "../Global.h"
#include "../VFileIndex.h"
#include "../Statistics.h"

#include <sys/time.h>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <iomanip>

using namespace std;

void randstr(char *s, const int len) {
    static const char alphanum[] =
//        "0123456789"
//        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; i++) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len] = '\0';
}

int main()
{
    VFileIndex index;
    char key[MAX_KVTSIZE];
    off_t off1, off2;
#if DBGLVL > 0
    off_t off3, off4;
#endif
    struct timeval tv;
    int num_keys, maxkeysize;
    #define N 7
    char *terms[N]   = {(char *)"b", (char *)"d", (char *)"g", (char *)"k", (char *)"m", (char *)"p", (char *)"x"};
    off_t offsets[N] = {  0,  10,  20,  30,  40,  50,  60};

    global_stats_init(); // avoid assertion error...

    gettimeofday(&tv, NULL);
// tv.tv_usec = 438701;
    cout << "seed: " << tv.tv_usec << endl;

    srand(tv.tv_usec);
    num_keys = rand() % 50000;
    maxkeysize = 3;
    cout << "num keys to be searched: " << num_keys << endl;

    //========================================================
    // insert values
    //========================================================
    srand(tv.tv_usec);

    for (int i = 0; i < N; i++) {
        index.add((char *)terms[i], offsets[i]);
    }
    index.set_vfilesize(offsets[N-1] + 3);

    //========================================================
    // check values
    //========================================================
    for (int i = 0; i < N; i++) {
        assert(index.search(terms[i], &off1, &off2));
        sprintf(key, "%s%s", terms[i], "00");
        if (i < N - 1) {
            assert(index.search(key, &off3, &off4));
            assert(off1 == off3 && off2 == off4);
        } else {
            assert(!(index.search(key, &off3, &off4)));
        }
    }

    assert(!index.search((char *)"a", &off1, &off2));
    assert(!index.search((char *)"zzz", &off1, &off2));

    for (int i = 0; i < num_keys; i++) {
        randstr(key, (int)(rand() % maxkeysize) + 1);
        if (index.search(key, &off1, &off2)) {
            assert(strcmp(key, terms[0]) >= 0 && strcmp(key, terms[N-1]) <= 0);
        } else {
            assert(strcmp(key, terms[0]) < 0 || strcmp(key, terms[N-1]) > 0);
        }
    }

#if 0
    //========================================================
    // print values
    //========================================================
    srand(tv.tv_usec);

    for (int i = 0; i < N; i++) {
        cout << setw(maxkeysize+1) << right << terms[i]; fflush(stdout);
        if (index.search(terms[i], &off1, &off2)) {
            cout << "(" << off1 << ", " << off2 << ")" << endl;
        } else {
            cout << "not found" << endl;
        }

        sprintf(key, "%s%s", terms[i], "00");
        cout << setw(maxkeysize+1) << left << key; fflush(stdout);
        if (index.search(key, &off1, &off2)) {
            cout << "(" << off1 << ", " << off2 << ")" << endl;
        } else {
            cout << "not found" << endl;
        }
    }

    for (int i = 0; i < num_keys; i++) {
        randstr(key, (int)(rand() % maxkeysize) + 1);
        cout << setw(maxkeysize+1) << left << key; fflush(stdout);
        if (index.search(key, &off1, &off2)) {
            cout << "(" << off1 << ", " << off2 << ")" << endl;
        } else {
            cout << "not found" << endl;
        }
    }
#endif

    cout << "Everything ok!" << endl;

    return EXIT_SUCCESS;
}
