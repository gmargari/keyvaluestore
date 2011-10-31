#include "../Global.h"
#include "../DiskFile.h"
#include "../DiskFileInputStream.h"
#include "../PriorityInputStream.h"
#include "../Statistics.h"

#include <cstdio>
#include <cstdlib>

using std::vector;

int main(int argc, char **argv)
{
    const char *key, *value;
    char prev_key[MAX_KVTSIZE];
    vector<DiskFile *> diskfiles;
    vector<InputStream *> istreams;
    PriorityInputStream *pistream;
    uint64_t timestamp;

    if (argc < 2) {
        printf("Syntax: %s <fsim-file1> ... <fsim-fileN>\n", argv[0]);
        return EXIT_FAILURE;
    }

    global_stats_init(); // avoid assertion error...

    for (int i = 0; i < argc - 1; i++) {
        diskfiles.push_back(new DiskFile());;
        diskfiles.back()->open_existing(argv[i+1]);
        istreams.push_back(new DiskFileInputStream(diskfiles.back(), MERGE_BUFSIZE));
    }

    pistream = new PriorityInputStream(istreams);
    pistream->set_key_range(NULL, NULL);
    prev_key[0] = '\0';
    while (pistream->read(&key, &value, &timestamp)) {
        if (strcmp(prev_key, key) != 0) {
            printf("[%s] [%s] [%Lu]\n", key, value, timestamp);
        }
        strcpy(prev_key, key);
    }

    for (unsigned int i = 0; i < istreams.size(); i++) {
        delete istreams[i];
    }
    for (unsigned int i = 0; i < diskfiles.size(); i++) {
        delete diskfiles[i];
    }
    delete pistream;

    return EXIT_SUCCESS;
}
