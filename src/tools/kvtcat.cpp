#include "../KVTDiskFile.h"
#include "../KVTDiskFileInputStream.h"

#include <cstdio>
#include <cstdlib>

int main(int argc, char **argv)
{
    const char *key, *value;
    KVTDiskFile *kvtdiskfile;
    KVTDiskFileInputStream *istream;
    uint64_t timestamp;

    if (argc != 2) {
        printf("Syntax: %s <kvtdiskfile>\n", argv[0]);
        return EXIT_FAILURE;
    }

    kvtdiskfile = new KVTDiskFile();
    kvtdiskfile->open_existing(argv[1]);
    istream = new KVTDiskFileInputStream(kvtdiskfile);
    while (istream->read(&key, &value, &timestamp)) {
        printf("[%s] [%s] [%Ld]\n", key, value, timestamp);
    }
    delete istream;
    delete kvtdiskfile;

    return EXIT_SUCCESS;
}
