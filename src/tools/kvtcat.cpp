#include "../KVTDiskFile.h"
#include "../KVTDiskFileInputStream.h"

#include <cstdio>
#include <cstdlib>

int main(int argc, char **argv)
{
    const char *key, *value;
    KVTDiskFile *kvtdiskfile;
    KVTDiskFileInputStream *istream;

    if (argc != 2) {
        printf("Syntax: %s <kvtdiskfile>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    kvtdiskfile = new KVTDiskFile();
    kvtdiskfile->open_existing(argv[1]);
    istream = new KVTDiskFileInputStream(kvtdiskfile);
    while (istream->read(&key, &value)) {
        printf("[%s] [%s]\n", key, value);
    }
    delete istream;
    delete kvtdiskfile;
}
