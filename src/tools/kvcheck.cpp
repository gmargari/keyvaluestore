#include "../Global.h"
#include "../KVDiskFile.h"
#include "../KVDiskFileInputStream.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, char **argv)
{
    const char *key, *value;
    char *prev_key;
    KVDiskFile *kvdiskfile;
    KVDiskFileInputStream *istream;

    if (argc != 2) {
        printf("Syntax: %s <kvdiskfile>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    prev_key = (char *)malloc(MAX_KVSIZE);
    prev_key[0] = '\0';
    kvdiskfile = new KVDiskFile();
    kvdiskfile->open_existing(argv[1]);
    istream = new KVDiskFileInputStream(kvdiskfile);
    while (istream->read(&key, &value)) {
        if (strcmp(prev_key, key) > 0) { // TODO: when we'll add timestamps, also check timestamps
            
            printf("Error: prev_key: %s > cur_key: %s\n", prev_key, key);
            exit(EXIT_FAILURE);
        }
        strcpy(prev_key, key);
    }
    free(prev_key);
    delete istream;
    delete kvdiskfile;
    
    printf("OK!\n");
}
