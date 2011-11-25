#include "../Global.h"
#include "../DiskFile.h"
#include "../DiskFileInputStream.h"
#include "../Statistics.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace std;

int main(int argc, char **argv)
{
    const char *key, *value;
    char *prev_key;
    DiskFile *diskfile;
    DiskFileInputStream *istream;
    uint64_t timestamp;
    int cmp;

    if (argc != 2) {
        cout << "Syntax: " << argv[0] << " <diskfile>" << endl;
        return EXIT_FAILURE;
    }

    global_stats_init(); // avoid assertion error...

    prev_key = (char *)malloc(MAX_KVTSIZE);
    prev_key[0] = '\0';
    diskfile = new DiskFile();
    diskfile->open_existing(argv[1]);
    istream = new DiskFileInputStream(diskfile, MERGE_BUFSIZE);
    istream->set_key_range(NULL, NULL);
    while (istream->read(&key, &value, &timestamp)) {
        if ((cmp = strcmp(prev_key, key)) > 0) {
            cout << "Error: prev_key: " << prev_key << " > cur_key: " << key << endl;
            return EXIT_FAILURE;
        }
        else if (cmp == 0) {
            cout << "Error: prev_key: " << prev_key << " == cur_key: "  << key << endl;
            return EXIT_FAILURE;
        }
        strcpy(prev_key, key);
    }
    free(prev_key);
    delete istream;
    delete diskfile;

    cout << "OK!" << endl;
    return EXIT_SUCCESS;
}
