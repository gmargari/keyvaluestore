#include "../Global.h"
#include "../DiskFile.h"
#include "../DiskFileInputStream.h"
#include "../Statistics.h"

#include <stdlib.h>
#include <iostream>

using namespace std;

int main(int argc, char **argv) {
    const char *key, *value;
    uint32_t keylen, valuelen;
    DiskFile *diskfile;
    DiskFileInputStream *istream;
    uint64_t timestamp;

    if (argc != 2) {
        cout << "Syntax: " << argv[0] << " <diskfile>" << endl;
        return EXIT_FAILURE;
    }

    global_stats_init(); // avoid assertion error...

    diskfile = new DiskFile();
    diskfile->open_existing(argv[1]);
    istream = new DiskFileInputStream(diskfile, MERGE_BUFSIZE);
    while (istream->read(&key, &keylen, &value, &valuelen, &timestamp)) {
        cout << "[" << key << "] [" << value << "] [" << timestamp << "]" << endl;
    }
    delete istream;
    delete diskfile;

    return EXIT_SUCCESS;
}
