#include "../Global.h"
#include "../KVTDiskFile.h"
#include "../KVTDiskFileInputStream.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, char **argv)
{
    const char *key, *value;
    char *prev_key;
    KVTDiskFile *kvtdiskfile;
    KVTDiskFileInputStream *istream;
    uint64_t timestamp, prev_timestamp = 0;
    int cmp;

    if (argc != 2) {
        printf("Syntax: %s <kvtdiskfile>\n", argv[0]);
        return EXIT_FAILURE;
    }

    prev_key = (char *)malloc(MAX_KVTSIZE);
    prev_key[0] = '\0';
    kvtdiskfile = new KVTDiskFile();
    kvtdiskfile->open_existing(argv[1]);
    istream = new KVTDiskFileInputStream(kvtdiskfile, MERGE_BUFSIZE);
    while (istream->read(&key, &value, &timestamp)) {
        if ((cmp = strcmp(prev_key, key)) > 0) {
            printf("Error: prev_key: %s > cur_key: %s\n", prev_key, key);
            return EXIT_FAILURE;
        }
        else if (cmp == 0) {
            printf("Error: prev_key: %s == cur_key: %s\n", prev_key, key);
            return EXIT_FAILURE;
        }
//         // this was used when duplicate keys were permitted
//         else if (cmp == 0 && prev_timestamp > timestamp) {
//             printf("Error: prev_timestamp: %Ld > cur_timestamp: %Ld\n", prev_timestamp, timestamp);
//             return EXIT_FAILURE;
//         }
        strcpy(prev_key, key);
        prev_timestamp = timestamp;
    }
    free(prev_key);
    delete istream;
    delete kvtdiskfile;

    printf("OK!\n");
    return EXIT_SUCCESS;
}
