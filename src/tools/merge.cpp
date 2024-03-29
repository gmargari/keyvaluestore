#include "../Global.h"
#include "../DiskFile.h"
#include "../DiskFileInputStream.h"
#include "../PriorityInputStream.h"
#include "../Statistics.h"

#include <iostream>
#include <stdlib.h>

using namespace std;
using std::vector;

int main(int argc, char **argv) {
    Slice key, value;
    char prev_key[MAX_KEY_SIZE + 1];
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
        istreams.push_back(new DiskFileInputStream(diskfiles.back()));
    }

    pistream = new PriorityInputStream(istreams);
    prev_key[0] = '\0';
    while (pistream->read(&key, &value, &timestamp)) {
        if (strcmp(prev_key, key.data()) != 0) {
            cout << "[" << key.data() << "] ["  << value.data() << "] [" << timestamp << "]" << endl;
        }
        memcpy(prev_key, key.data(), key.size() + 1);
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
