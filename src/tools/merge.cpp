#include "../Global.h"
#include "../DiskFile.h"
#include "../DiskFileInputStream.h"
#include "../PriorityInputStream.h"
#include "../Statistics.h"

#include <iostream>
#include <cstdlib>

using namespace std;
using std::vector;

int main(int argc, char **argv)
{
    const char *key, *value;
    uint32_t keylen, valuelen;
    char prev_key[MAX_KVTSIZE];
    vector<DiskFile *> diskfiles;
    vector<InputStream *> istreams;
    PriorityInputStream *pistream;
    uint64_t timestamp;

    if (argc < 2) {
        cout << "Syntax: " << argv[0] << " <diskfile1> ... <diskfileN>" << endl;
        return EXIT_FAILURE;
    }

    global_stats_init(); // avoid assertion error...

    for (int i = 0; i < argc - 1; i++) {
        diskfiles.push_back(new DiskFile());;
        diskfiles.back()->open_existing(argv[i+1]);
        istreams.push_back(new DiskFileInputStream(diskfiles.back(), MERGE_BUFSIZE));
    }

    pistream = new PriorityInputStream(istreams);
    prev_key[0] = '\0';
    while (pistream->read(&key, &keylen, &value, &valuelen, &timestamp)) {
        if (strcmp(prev_key, key) != 0) {
            cout << "[" << key << "] ["  << value << "] [" << timestamp << "]" << endl;
        }
        memcpy(prev_key, key, keylen + 1);
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
