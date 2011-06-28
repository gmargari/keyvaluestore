#include "../Global.h"
#include "../DiskFile.h"
#include "../DiskFileInputStream.h"
#include "../PriorityInputStream.h"

#include <cstdio>
#include <cstdlib>

using std::vector;

int main(int argc, char **argv)
{
    const char *key, *value;
    vector<DiskFile *> diskfiles;
    vector<InputStream *> istreams;
    PriorityInputStream *priority_istream;
    uint64_t timestamp;

    if (argc < 2) {
        printf("Syntax: %s <diskfile1> ... <diskfileN>\n", argv[0]);
        return EXIT_FAILURE;
    }

    for (int i = 0; i < argc - 1; i++) {
        diskfiles.push_back(new DiskFile());;
        diskfiles.back()->open_existing(argv[i+1]);
        istreams.push_back(new DiskFileInputStream(diskfiles.back(), MERGE_BUFSIZE));
    }

    priority_istream = new PriorityInputStream(istreams);
    while (priority_istream->read(&key, &value, &timestamp)) {
        printf("[%s] [%s] [%Ld]\n", key, value, timestamp);
    }

    for (unsigned int i = 0; i < istreams.size(); i++) {
        delete istreams[i];
    }
    for (unsigned int i = 0; i < diskfiles.size(); i++) {
        delete diskfiles[i];
    }
    delete priority_istream;

    return EXIT_SUCCESS;
}
