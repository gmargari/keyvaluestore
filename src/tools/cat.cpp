#include "../Global.h"
#include "../DiskFile.h"
#include "../DiskFileInputStream.h"
#include "../Statistics.h"

#include <cstdio>
#include <cstdlib>

int main(int argc, char **argv)
{
    const char *key, *value;
    DiskFile *diskfile;
    DiskFileInputStream *istream;
    uint64_t timestamp;

    if (argc != 2) {
        printf("Syntax: %s <diskfile>\n", argv[0]);
        return EXIT_FAILURE;
    }

    global_stats_init(); // avoid assertion error...

    diskfile = new DiskFile();
    diskfile->open_existing(argv[1]);
    istream = new DiskFileInputStream(diskfile, MERGE_BUFSIZE);
    while (istream->read(&key, &value, &timestamp)) {
        printf("[%s] [%s] [%Ld]\n", key, value, timestamp);
    }
    delete istream;
    delete diskfile;

    return EXIT_SUCCESS;
}
