#include "../VFileIndex.h"

#include <sys/time.h>
#include <cstdlib>
#include <cassert>

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


int main(void)
{
    VFileIndex index;
    char key[MAX_KVTSIZE];
    off_t off1, off2, off3, off4;
    struct timeval tv;
    int num_keys, maxkeysize;
    #define N 7
    char *terms[N]   = {(char *)"b", (char *)"d", (char *)"g", (char *)"k", (char *)"m", (char *)"p", (char *)"x"};
    off_t offsets[N] = {  0,  10,  20,  30,  40,  50,  60};

    gettimeofday(&tv, NULL);
// tv.tv_usec = 438701;
    printf("seed: %ld\n", tv.tv_usec);

    srand(tv.tv_usec);
    num_keys = rand() % 50000;
    maxkeysize = 3;
    printf("num keys to be searched: %d\n", num_keys);

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
        printf("%-*s ", maxkeysize+1, terms[i]); fflush(stdout);
        if (index.search(terms[i], &off1, &off2)) {
            printf("(%Ld, %Ld)\n", off1, off2);
        } else {
            printf("not found\n");
        }

        sprintf(key, "%s%s", terms[i], "00");
        printf("%-*s ", maxkeysize+1, key); fflush(stdout);
        if (index.search(key, &off1, &off2)) {
            printf("(%Ld, %Ld)\n", off1, off2);
        } else {
            printf("not found\n");
        }
    }

    for (int i = 0; i < num_keys; i++) {
        randstr(key, (int)(rand() % maxkeysize) + 1);
        printf("%-*s ", maxkeysize+1, key); fflush(stdout);
        if (index.search(key, &off1, &off2)) {
            printf("(%Ld, %Ld)\n", off1, off2);
        } else {
            printf("not found\n");
        }
    }
#endif

    printf("Everything ok!\n");

    return EXIT_SUCCESS;
}
