#include "../Global.h"
#include "../DiskFile.h"
#include "../DiskFileInputStream.h"
#include "../Statistics.h"

#include <stdlib.h>
#include <iostream>

using namespace std;

int main(int argc, char **argv) {
    Slice key, value;
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
    istream = new DiskFileInputStream(diskfile);
    while (istream->read(&key, &value, &timestamp)) {
        cout << "[" << key.data() << "] [" << value.data() << "] [" << timestamp << "]" << endl;
    }
    delete istream;
    delete diskfile;

    return EXIT_SUCCESS;
}
