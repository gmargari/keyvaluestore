#include "../Global.h"
#include "../VFile.h"
#include "../Statistics.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <iostream>

using namespace std;

int main(void)
{
    VFile f;
    char *buf = (char *)malloc(mb2b(20));
    uint64_t bytes_left, num;
    off_t offs = 0;
    char filepath[1000];

    global_stats_init(); // avoid assertion error...

    memset(buf, 0, mb2b(20));

    // open()            -->   size: 0MB, offset: 0MB
    sprintf(filepath, "%s/vfiletest", ROOT_DIR);
    f.fs_open_new(filepath);
    assert(f.fs_size() == 0);
    assert(offs == 0);

    //==============[ code using truncate ]==================/
    f.fs_truncate(mb2b(0));
    offs = 0;
    assert(f.fs_size() == mb2b(0));
    f.fs_truncate(mb2b(6));
    offs = mb2b(6);
    offs += f.fs_append(buf, mb2b(4));
    assert(f.fs_size() == mb2b(10));
    assert(offs == (off_t)mb2b(10));
    f.fs_truncate(mb2b(0));
    offs = mb2b(0);
    assert(f.fs_size() == mb2b(0));
    assert(offs == (off_t)mb2b(0));
    f.fs_truncate(mb2b(1));
    offs = mb2b(1);
    assert(f.fs_size() == mb2b(1));
    assert(offs == (off_t)mb2b(1));
    f.fs_truncate(mb2b(2));
    offs = mb2b(2);
    assert(f.fs_size() == mb2b(2));
    assert(offs == (off_t)mb2b(2));
    f.fs_truncate(mb2b(0));
    offs = mb2b(0);
    assert(f.fs_size() == mb2b(0));
    assert(offs == (off_t)mb2b(0));
    f.fs_truncate(mb2b(10));
    offs = mb2b(10);
    assert(f.fs_size() == mb2b(10));
    assert(offs == (off_t)mb2b(10));
    f.fs_truncate(mb2b(0));
    offs = mb2b(0);
    assert(f.fs_size() == mb2b(0));
    assert(offs == (off_t)mb2b(0));
    f.fs_truncate(mb2b(11));
    offs = mb2b(11);
    assert(f.fs_size() == mb2b(11));
    assert(offs == (off_t)mb2b(11));

    //==============[ code using write & truncate ]==================/
    f.fs_truncate(mb2b(0));
    offs = mb2b(0);
    assert(f.fs_size() == mb2b(0));
    assert(offs == (off_t)mb2b(0));

    // write 3MB         -->   size: 3MB, offset: 3MB
    offs += f.fs_append(buf, mb2b(3));
    assert(f.fs_size() == mb2b(3));
    assert(offs == (off_t)mb2b(3));

    // write 3MB         -->   size: 6MB, offset: 6MB
    offs += f.fs_append(buf, mb2b(3));
    assert(f.fs_size() == mb2b(6));
    assert(offs == (off_t)mb2b(6));

    // write 1MB         -->   size: 7MB, offset: 7MB
    offs += f.fs_append(buf, mb2b(1));
    assert(f.fs_size() == mb2b(7));
    assert(offs == (off_t)mb2b(7));

    // write 3MB        -->   size: 10MB, offset: 10MB
    offs += f.fs_append(buf, mb2b(3));
    assert(f.fs_size() == mb2b(10));
    assert(offs == (off_t)mb2b(10));

    // truncate to 4MB   -->   size: 4MB, offset: 4MB
    f.fs_truncate(mb2b(4));
    offs = mb2b(4);
    assert(f.fs_size() == mb2b(4)); // NOTE: normally, truncate does not change file offset. our implementation sets offset to file size after truncating
    assert(offs == (off_t)mb2b(4));

    // truncate to 0MB   -->   size: 0MB, offset: 10MB
    f.fs_truncate(0);
    offs = 0;
    assert(f.fs_size() == 0);
    assert(offs == (off_t)0);

    // truncate to 4MB   -->   size: 4MB, offset: 4MB
    f.fs_truncate(mb2b(4));
    offs = mb2b(4);
    assert(f.fs_size() == mb2b(4));
    assert(offs == (off_t)mb2b(4));


    // truncate to 0 length and write 10MB
    f.fs_truncate(0);
    offs = 0;
    offs += f.fs_append(buf, mb2b(10));
    assert(f.fs_size() == mb2b(10));
    assert(offs == (off_t)mb2b(10));

    // write 3MB
    offs += f.fs_append(buf, mb2b(3));
    assert(f.fs_size() == mb2b(13));
    assert(offs == (off_t)mb2b(13));

    // write 3MB
    offs += f.fs_append(buf, mb2b(3));
    assert(f.fs_size() == mb2b(16));
    assert(offs == (off_t)mb2b(16));

    // write 3MB
    offs += f.fs_append(buf, mb2b(4));
    assert(f.fs_size() == mb2b(20));
    assert(offs == (off_t)mb2b(20));

    // truncate to 0 length
    f.fs_truncate(0);
    offs = 0;

    // write 5MB
    offs += f.fs_append(buf, mb2b(5));
    assert(f.fs_size() == mb2b(5));
    assert(offs == (off_t)mb2b(5));

    // write 5MB
    offs += f.fs_append(buf, mb2b(5));
    assert(f.fs_size() == mb2b(10));
    assert(offs == (off_t)mb2b(10));

    // write 10MB
    offs += f.fs_append(buf, mb2b(10));
    assert(f.fs_size() == mb2b(20));
    assert(offs == (off_t)mb2b(20));

    // write 10MB
    offs += f.fs_append(buf, mb2b(10));
    assert(f.fs_size() == mb2b(30));
    assert(offs == (off_t)mb2b(30));

    // truncate to 0 bytes
    f.fs_truncate(0);
    offs = 0;
    assert(f.fs_size() == mb2b(0));
    assert(offs == (off_t)mb2b(0));

    // truncate to 0 bytes, write 60MB by writing each time 3MB
    f.fs_truncate(0);
    offs = 0;
    for (bytes_left = mb2b(60); bytes_left;) {
        num = f.fs_append(buf, mb2b(3));
        bytes_left -= num;
        offs += num;
    }
    assert(f.fs_size() == mb2b(60));
    assert(offs == (off_t)mb2b(60));

    // write 135MB
    free(buf);
    buf = (char *)malloc(mb2b(60));
    assert(buf);
    memset(buf, 0, mb2b(60));
    f.fs_truncate(0);
    offs = 0;
    for (bytes_left = mb2b(60); bytes_left;) {
        bytes_left -= offs += f.fs_append(buf, bytes_left);
    }
    assert(f.fs_size() == mb2b(60));
    assert(offs == (off_t)mb2b(60));

    //==============[ code using truncate, write & read ]==================/
    f.fs_truncate(mb2b(15));
    offs = mb2b(15);
    offs = 0;
    offs += f.fs_pread(buf, mb2b(5), offs);
    assert(f.fs_size() == mb2b(15));
    assert(offs == (off_t)mb2b(5));
    offs += f.fs_pread(buf, mb2b(5), offs);
    assert(f.fs_size() == mb2b(15));
    assert(offs == (off_t)mb2b(10));
    offs += f.fs_pread(buf, mb2b(2), offs);
    assert(f.fs_size() == mb2b(15));
    assert(offs == (off_t)mb2b(12));
    offs += f.fs_pread(buf, mb2b(3), offs);
    assert(f.fs_size() == mb2b(15));
    assert(offs == (off_t)mb2b(15));
    offs += f.fs_append(buf, mb2b(3));
    assert(f.fs_size() == mb2b(18));
    offs -= mb2b(3);
    offs += f.fs_pread(buf, mb2b(3), offs);
    assert(f.fs_size() == mb2b(18));
    assert(offs == (off_t)mb2b(18));

    free(buf);

    cout << "Everything ok!" << endl;

    return EXIT_SUCCESS;
}
