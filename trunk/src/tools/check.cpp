#include "../Global.h"
#include "../DiskFile.h"
#include "../DiskFileInputStream.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, char **argv)
{
    const char *key, *value;
    char *prev_key;
    DiskFile *diskfile;
    DiskFileInputStream *istream;
    uint64_t timestamp, prev_timestamp = 0;
    int cmp;

    if (argc != 2) {
        printf("Syntax: %s <diskfile>\n", argv[0]);
        return EXIT_FAILURE;
    }

    prev_key = (char *)malloc(MAX_KVTSIZE);
    prev_key[0] = '\0';
    diskfile = new DiskFile();
    diskfile->open_existing(argv[1]);
    istream = new DiskFileInputStream(diskfile, MERGE_BUFSIZE);
    istream->set_key_range(NULL, NULL);
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
    delete diskfile;

    printf("OK!\n");
    return EXIT_SUCCESS;
}
