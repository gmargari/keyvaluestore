#include "../Global.h"
#include "../KVTDiskFile.h"
#include "../KVTDiskFileInputStream.h"
#include "../KVTPriorityInputStream.h"

#include <cstdio>
#include <cstdlib>

using std::vector;

int main(int argc, char **argv)
{
    const char *key, *value;
    vector<KVTDiskFile *> kvtdiskfiles;
    vector<KVTInputStream *> istreams;
    KVTPriorityInputStream *priority_istream;
    uint64_t timestamp;

    if (argc < 2) {
        printf("Syntax: %s <kvtdiskfile1> ... <kvtdiskfileN>\n", argv[0]);
        return EXIT_FAILURE;
    }

    for (int i = 0; i < argc - 1; i++) {
        kvtdiskfiles.push_back(new KVTDiskFile());;
        kvtdiskfiles.back()->open_existing(argv[i+1]);
        istreams.push_back(new KVTDiskFileInputStream(kvtdiskfiles.back(), MERGE_BUFSIZE));
    }

    priority_istream = new KVTPriorityInputStream(istreams);
    while (priority_istream->read(&key, &value, &timestamp)) {
        printf("[%s] [%s] [%Ld]\n", key, value, timestamp);
    }

    for (unsigned int i = 0; i < istreams.size(); i++) {
        delete istreams[i];
    }
    for (unsigned int i = 0; i < kvtdiskfiles.size(); i++) {
        delete kvtdiskfiles[i];
    }
    delete priority_istream;

    return EXIT_SUCCESS;
}
