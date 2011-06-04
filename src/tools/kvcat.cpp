#include "../KVDiskFile.h"
#include "../KVDiskFileInputStream.h"

#include <cstdio>
#include <cstdlib>

int main(int argc, char **argv)
{
    const char *key, *value;
    KVDiskFile *kvdiskfile;
    KVDiskFileInputStream *istream;

    if (argc != 2) {
        printf("Syntax: %s <kvdiskfile>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    kvdiskfile = new KVDiskFile();
    kvdiskfile->open_existing(argv[1]);
    istream = new KVDiskFileInputStream(kvdiskfile);
    while (istream->read(&key, &value)) {
        printf("[%s] [%s]\n", key, value);
    }
    delete istream;
    delete kvdiskfile;
}
