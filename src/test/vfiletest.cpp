#include "../Global.h"
#include "../VFile.h"
#include "../Statistics.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

int main(void)
{
    VFile f;
    void *buf = malloc(mb2b(20));
    uint64_t bytes_left;

    global_stats_init(); // avoid assertion error...

    memset(buf, 0, mb2b(20));

    // open()            -->   size: 0MB, offset: 0MB
dbg();
    f.fs_open_new((char *)"vfiletest");
dbg();
    assert(f.fs_size() == 0);
dbg();
    assert(f.fs_tell() == 0);
dbg();

    //==============[ code using truncate ]==================/
dbg();
    f.fs_truncate(mb2b(0));
    assert(f.fs_size() == mb2b(0));
    assert(f.fs_tell() == (off_t)mb2b(0));
dbg();
    f.fs_truncate(mb2b(6));
    f.fs_seek(0, SEEK_SET);
    assert(f.fs_tell() == (off_t)mb2b(0));
dbg();
    f.fs_write(buf, mb2b(4));
    assert(f.fs_size() == mb2b(6));
    assert(f.fs_tell() == (off_t)mb2b(4));
dbg();
    f.fs_truncate(mb2b(0));
    assert(f.fs_size() == mb2b(0));
    assert(f.fs_tell() == (off_t)mb2b(0));
dbg();
    f.fs_truncate(mb2b(1));
    assert(f.fs_size() == mb2b(1));
    assert(f.fs_tell() == (off_t)mb2b(1));
dbg();
    f.fs_truncate(mb2b(2));
    assert(f.fs_size() == mb2b(2));
    assert(f.fs_tell() == (off_t)mb2b(2));
dbg();
    f.fs_truncate(mb2b(0));
    assert(f.fs_size() == mb2b(0));
    assert(f.fs_tell() == (off_t)mb2b(0));
dbg();
    f.fs_truncate(mb2b(10));
    assert(f.fs_size() == mb2b(10));
    assert(f.fs_tell() == (off_t)mb2b(10));
dbg();
    f.fs_truncate(mb2b(0));
    assert(f.fs_size() == mb2b(0));
    assert(f.fs_tell() == (off_t)mb2b(0));
dbg();
    f.fs_truncate(mb2b(11));
    assert(f.fs_size() == mb2b(11));
    assert(f.fs_tell() == (off_t)mb2b(11));

    //==============[ code using write & truncate ]==================/
dbg();
    f.fs_truncate(mb2b(0));
    assert(f.fs_size() == mb2b(0));
    assert(f.fs_tell() == (off_t)mb2b(0));

// write 3MB         -->   size: 3MB, offset: 3MB
    f.fs_write(buf, mb2b(3));
    assert(f.fs_size() == mb2b(3));
    assert(f.fs_tell() == (off_t)mb2b(3));
dbg();
    // write 3MB         -->   size: 6MB, offset: 6MB
    f.fs_write(buf, mb2b(3));
    assert(f.fs_size() == mb2b(6));
    assert(f.fs_tell() == (off_t)mb2b(6));
dbg();
    // seek -2MB         -->   size: 6MB, offset: 4MB
    f.fs_seek(mb2b(-2), SEEK_CUR);
    assert(f.fs_size() == mb2b(6));
    assert(f.fs_tell() == (off_t)mb2b(4));
dbg();
    // write 3MB         -->   size: 7MB, offset: 7MB
    f.fs_write(buf, mb2b(3));
    assert(f.fs_size() == mb2b(7));
    assert(f.fs_tell() == (off_t)mb2b(7));
dbg();
    // write 3MB        -->   size: 10MB, offset: 10MB
    f.fs_write(buf, mb2b(3));
    assert(f.fs_size() == mb2b(10));
    assert(f.fs_tell() == (off_t)mb2b(10));
dbg();
    // truncate to 4MB   -->   size: 4MB, offset: 4MB
    f.fs_truncate(mb2b(4));
    assert(f.fs_size() == mb2b(4)); // NOTE: normally, truncate does not change file offset. our implementation sets offset to file size after truncating
    assert(f.fs_tell() == (off_t)mb2b(4));
dbg();
    // truncate to 0MB   -->   size: 0MB, offset: 10MB
    f.fs_truncate(0);
    assert(f.fs_size() == 0);
    assert(f.fs_tell() == (off_t)0);
dbg();
    // truncate to 4MB   -->   size: 4MB, offset: 4MB
    f.fs_truncate(mb2b(4));
    assert(f.fs_size() == mb2b(4));
    assert(f.fs_tell() == (off_t)mb2b(4));
dbg();
    // rewind            -->   size: 4MB, offset: 0MB
    f.fs_rewind();
    assert(f.fs_size() == mb2b(4));
    assert(f.fs_tell() == (off_t)0);
dbg();

    // truncate to MAXFILESIZE
    f.fs_truncate(MAX_FILE_SIZE);
    assert(f.fs_size() == MAX_FILE_SIZE);
dbg();
    // seek to 1MB
    f.fs_seek(mb2b(1), SEEK_SET);
    assert(f.fs_tell() == (off_t)mb2b(1));
    assert(f.fs_size() == MAX_FILE_SIZE);
dbg();
    // seek to 0
    f.fs_seek(mb2b(-1), SEEK_CUR);
    assert(f.fs_tell() == (off_t)0);
    assert(f.fs_size() == MAX_FILE_SIZE);
dbg();
    // seek to MAX_FILE_SIZE - 1
    f.fs_seek(mb2b(-1), SEEK_END);
    assert(f.fs_tell() == (off_t)(MAX_FILE_SIZE - mb2b(1)));
    assert(f.fs_size() == MAX_FILE_SIZE);
dbg();
    // write 1MB
    f.fs_write(buf, mb2b(1));
    assert(f.fs_size() == MAX_FILE_SIZE);
    assert(f.fs_tell() == (off_t)MAX_FILE_SIZE);
dbg();
    // truncate to 0 length and write 10MB
    f.fs_truncate(0);
    f.fs_write(buf, mb2b(10));
    assert(f.fs_size() == mb2b(10));
    assert(f.fs_tell() == (off_t)mb2b(10));
dbg();
    // write 3MB
    f.fs_write(buf, mb2b(3));
    assert(f.fs_size() == mb2b(13));
    assert(f.fs_tell() == (off_t)mb2b(13));
dbg();
    // write 3MB
    f.fs_write(buf, mb2b(3));
    assert(f.fs_size() == mb2b(16));
    assert(f.fs_tell() == (off_t)mb2b(16));
dbg();
    // write 3MB
    f.fs_write(buf, mb2b(4));
    assert(f.fs_size() == mb2b(20));
    assert(f.fs_tell() == (off_t)mb2b(20));
dbg();
    // seek -1MB, write 1MB
    f.fs_seek(mb2b(-1), SEEK_CUR);
    f.fs_write(buf, mb2b(1));
    assert(f.fs_size() == mb2b(20));
    assert(f.fs_tell() == (off_t)mb2b(20));
dbg();
    // seek 0
    f.fs_seek(0, SEEK_SET);
    assert(f.fs_tell() == (off_t)mb2b(0));
dbg();
    f.fs_write(buf, mb2b(10));
    assert(f.fs_size() == mb2b(20));
    assert(f.fs_tell() == (off_t)mb2b(10));
dbg();
    // write 5MB
    f.fs_write(buf, mb2b(5));
    assert(f.fs_size() == mb2b(20));
    assert(f.fs_tell() == (off_t)mb2b(15));
dbg();
    // write 5MB
    f.fs_write(buf, mb2b(5));
    assert(f.fs_size() == mb2b(20));
    assert(f.fs_tell() == (off_t)mb2b(20));
dbg();
    // write 10MB
    f.fs_write(buf, mb2b(10));
    assert(f.fs_size() == mb2b(30));
    assert(f.fs_tell() == (off_t)mb2b(30));
dbg();
    // write 10MB
    f.fs_write(buf, mb2b(10));
    assert(f.fs_size() == mb2b(40));
    assert(f.fs_tell() == (off_t)mb2b(40));
dbg();
    // truncate to 0 bytes
    f.fs_truncate(0);
    assert(f.fs_size() == mb2b(0));
    assert(f.fs_tell() == (off_t)mb2b(0));
dbg();
    // truncate to 0 bytes, write 60MB by writing each time 3MB
    f.fs_truncate(0);
    for (bytes_left = mb2b(60); bytes_left;) {
        bytes_left -= f.fs_write(buf, mb2b(3));
        printf("bytes left: %Ld\n", bytes_left);
    }
    assert(f.fs_size() == mb2b(60));
    assert(f.fs_tell() == (off_t)mb2b(60));
dbg();
    // write 135MB
    free(buf);
    buf = malloc(mb2b(60));
    assert(buf);
    memset(buf, 0, mb2b(60));
    f.fs_truncate(0);
    for (bytes_left = mb2b(60); bytes_left;) {
        bytes_left -= f.fs_write(buf, bytes_left);
        printf("bytes left: %Ld\n", bytes_left);
    }
    assert(f.fs_size() == mb2b(60));
    assert(f.fs_tell() == (off_t)mb2b(60));

    //==============[ code using truncate, write & read ]==================/
    f.fs_truncate(mb2b(15));
    f.fs_rewind();
dbg();
    f.fs_read(buf, mb2b(5));
    assert(f.fs_size() == mb2b(15));
    assert(f.fs_tell() == (off_t)mb2b(5));
dbg();
    f.fs_read(buf, mb2b(5));
    assert(f.fs_size() == mb2b(15));
    assert(f.fs_tell() == (off_t)mb2b(10));
dbg();
    f.fs_read(buf, mb2b(2));
    assert(f.fs_size() == mb2b(15));
    assert(f.fs_tell() == (off_t)mb2b(12));
dbg();
    f.fs_read(buf, mb2b(3));
    assert(f.fs_size() == mb2b(15));
    assert(f.fs_tell() == (off_t)mb2b(15));
dbg();
    f.fs_write(buf, mb2b(3));
    f.fs_seek(mb2b(-3), SEEK_CUR);
    f.fs_read(buf, mb2b(3));
    assert(f.fs_size() == mb2b(18));
    assert(f.fs_tell() == (off_t)mb2b(18));

    free(buf);

    printf("Everything ok!\n");

    return EXIT_SUCCESS;
}