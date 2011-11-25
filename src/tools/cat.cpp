#include "../Global.h"
#include "../DiskFile.h"
#include "../DiskFileInputStream.h"
#include "../Statistics.h"

#include <cstdlib>
#include <iostream>

using namespace std;

int main(int argc, char **argv)
{
    const char *key, *value;
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
    istream->set_key_range(NULL, NULL);
    while (istream->read(&key, &value, &timestamp)) {
        cout << "[" << key << "] [" << value << "] [" << timestamp << "]" << endl;
    }
    delete istream;
    delete diskfile;

    return EXIT_SUCCESS;
}
