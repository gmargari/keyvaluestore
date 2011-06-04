#include "Global.h"
#include "VFile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <sys/time.h>
#include <stdint.h>

using namespace std;
#include <cstdio>

/*-------------------------------------------------------
 *                       VFile
 *-------------------------------------------------------*/
VFile::VFile(bool real_io)
{
    assert(sizeof(off_t) == 8); // ensure 64bit offsets

    // TODO: for now, each VFile has a separate simulator, and we CANNOT
    // get its stats. just trying if IO is ok...
    Sim = new Simulator();
    
    if (real_io) {
        Sim->set_simulation_mode(Simulator::SIMMODE_REAL_IO);
    } else {
        Sim->set_simulation_mode(Simulator::SIMMODE_SIMULATE_IO);
    }

    Size = 0;
    Offset = 0;
    Opened = -1;
    Cur = -1;    
}

/*-------------------------------------------------------
 *                       ~VFile
 *-------------------------------------------------------*/
VFile::~VFile(void)
{
    if (Sim->get_simulation_mode() == Simulator::SIMMODE_SIMULATE_IO) {

    } else {
        // TODO: currently, we do not delete files from disk...
//         fs_delete(); // if we performed real io, delete file(s) from disk
    }

    // TODO: for now, each VFile has a separate simulator, and we CANNOT
    // get its stats. just trying if IO is ok...
    delete Sim;

    for (int i = 0; i < (int)Names.size(); i++) {
        free(Names[i]);
        Names[i] = NULL;
    }
}

/*-------------------------------------------------------
 *                       fs_open
 *-------------------------------------------------------*/
bool VFile::fs_open(char *filename, bool open_existing)
{
    char *fname;

    fname = strdup(filename);
    assert(fname);
    printf("[DEBUG] VFile::fs_open [%s]\n", fname);

    if (Sim->get_simulation_mode() == Simulator::SIMMODE_SIMULATE_IO) {
        if (Opened) {
            printf("[ERROR] %s(): file already open\n", __FUNCTION__);
            exit(EXIT_FAILURE);
        }

        Opened = true;
        Names.push_back(fname);
        Size = 0;
        Offset = 0;
        Cur = 0;
        return true;
    } else {
        int fd, fdflag, fdmode;

        if (open_existing) {
            fdflag = O_RDWR;
        } else {
            fdflag = O_RDWR | O_CREAT;
        }
        fdmode = S_IRUSR | S_IWUSR;
        
        if ((fd = open(fname, fdflag, fdmode)) == -1) {
            printf("[ERROR] %s('%s')\n", __FUNCTION__, fname);
            perror("");
            return false;
        }

        Names.push_back(fname);
        Filedescs.push_back(fd);
        Cur = Filedescs.size() - 1;
        return true;
    }
}

/*-------------------------------------------------------
 *                   fs_open_existing
 *-------------------------------------------------------*/
bool VFile::fs_open_existing(char *filename)
{
    return fs_open(filename, true);
}

/*-------------------------------------------------------
 *                    fs_open_unique
 *-------------------------------------------------------*/
bool VFile::fs_open_unique(void)
{
    char *name = tempnam(TMPFILEDIR, TMPFILEPREFIX);
    fs_open(name);    
    free(name);
    return true;
}

/*-------------------------------------------------------
 *                      close
 *-------------------------------------------------------*/
void VFile::fs_close(void)
{
    assert(Cur != -1);
    if (Sim->get_simulation_mode() == Simulator::SIMMODE_SIMULATE_IO) {
        if (!Opened) {
            printf("[ERROR] %s(): file not open\n", __FUNCTION__);
            exit(EXIT_FAILURE);
        }

        Opened = false;
        Offset = 0;
    } else {
        for (int i = 0; i < (int)Filedescs.size(); i++) {
            if (close(Filedescs[i]) == -1) {
                printf("[ERROR] %s(): close('%s')\n", __FUNCTION__, Names[Cur]);
                perror("");
                exit(EXIT_FAILURE);
            }
        }
    }
}

/*-------------------------------------------------------
 *                       fs_read
 *-------------------------------------------------------*/
ssize_t VFile::fs_read(void *buf, size_t count)
{
    ssize_t bytes_read;

    assert(Cur != -1);
    
    if (Sim->get_simulation_mode() == Simulator::SIMMODE_SIMULATE_IO) {
        bytes_read = (size_t)min((uint64_t)count, (Size - Offset));
        Offset += bytes_read;
        assert(Offset <= Size);
    } else {
        size_t num;
        
        start_timing();
        for (bytes_read = 0; bytes_read < count; ) {
            if ((num = cur_fs_read(buf + bytes_read, count - bytes_read)) == 0) {
                break; // no bytes left in file
            }
            bytes_read += num;
        }
        end_timing();
    }

    Sim->inc_bytes_read(bytes_read);
    Sim->inc_ioops(1);
    
    return bytes_read;
}

/*-------------------------------------------------------
 *                       fs_write
 *-------------------------------------------------------*/
ssize_t VFile::fs_write(const void *buf, size_t count)
{
    assert(Cur != -1);
    
    if (Sim->get_simulation_mode() == Simulator::SIMMODE_SIMULATE_IO) {
        Offset += count;
        if (Offset > Size) {
            Size = Offset;
        }
    } else {
        size_t num;
        uint64_t bytes_written;

        start_timing();
        for (bytes_written = 0; bytes_written < count; ) {
            num = cur_fs_write(buf + bytes_written, count - bytes_written);
            bytes_written += num;
        }
        end_timing();
        assert(bytes_written == count);
    }

    Sim->inc_bytes_written(count);
    Sim->inc_ioops(1); // TODO: maybe ioops should be increased per internas_fs_write()
    
    return count;
}

/*-------------------------------------------------------
 *                       fs_seek
 *-------------------------------------------------------*/
off_t VFile::fs_seek(off_t offs, int whence)
{
    off_t newoffs;

    assert(Cur != -1);
    
    if (Sim->get_simulation_mode() == Simulator::SIMMODE_SIMULATE_IO) {
        if (whence == SEEK_SET) {
            Offset = offs;
        } else if (whence == SEEK_CUR) {
            Offset += offs;
        } else if (whence == SEEK_END) {
            Offset = Size + offs;
        }
        newoffs = Offset;
    } else {
        off_t abs_offs, offs_in_file;
        int fileno;

        /*
         * if seek is out of current file limits, check if there are previous
         * or next files in Filedescs and if we can get to that file. if yes,
         * advance to that file.
         */
        
        // calculate the absolute offset inside the virtual file
        if (whence == SEEK_SET) {
            abs_offs = offs;
        } else if (whence == SEEK_CUR) {
            abs_offs = Cur * MAX_FILE_SIZE + lseek(Filedescs[Cur], 0, SEEK_CUR) + offs;
        } else if (whence == SEEK_END) {
            abs_offs = fs_size() + offs;
        }

        if (abs_offs > (off_t)fs_size()) {
            printf("Haven't yet implemented seeking beyond end of file\n");
            exit(EXIT_FAILURE);
        }

        // calculate the file and the offset within the file
        fileno = abs_offs / MAX_FILE_SIZE;
        offs_in_file = abs_offs % MAX_FILE_SIZE;

        // example: if maxfilesize = 10bytes, and we:
        //    1) create a file
        //    2) write 10 bytes in file
        // then offset = 10, size = 10. In this case, fileno will be '1'
        // and offset will be '0', meaning that we have to seek to 0 byte of 
        // file 1. Instead we'll seek at fileno 0 at position 10.
        if (fileno > (int)Names.size() - 1) {
            fileno--;
            offs_in_file = MAX_FILE_SIZE;
        }
        
        // advance to that file
        Cur = fileno;
        
        // seek within that file
        if (lseek(Filedescs[Cur], offs_in_file, SEEK_SET) == (off_t)-1) {
            printf("[ERROR] %s(): lseek('%s', %Ld, %s)\n", __FUNCTION__, Names[Cur], offs, 
              (whence == SEEK_SET) ? "SEEK_SET" : ((whence == SEEK_CUR) ? "SEEK_CUR" : "SEEK_END"));
            perror("");
            exit(EXIT_FAILURE);
        }

        newoffs = abs_offs;
    }

    return newoffs;
}

/*-------------------------------------------------------
 *                      fs_tell
 *-------------------------------------------------------*/
off_t VFile::fs_tell(void)
{
    assert(Cur != -1);
    return fs_seek(0, SEEK_CUR);
}

/*-------------------------------------------------------
 *                      fs_rewind
 *-------------------------------------------------------*/
void VFile::fs_rewind(void)
{
    assert(Cur != -1);
    fs_seek(0, SEEK_SET);
}

/*-------------------------------------------------------
 *                      fs_truncate
 *-------------------------------------------------------*/
void VFile::fs_truncate(off_t length)
{
    assert(Cur != -1);
    
    if (Sim->get_simulation_mode() == Simulator::SIMMODE_SIMULATE_IO) {
        Size = length;        
    } else {
        int new_files, num;
        
        new_files = (int) ceil((float)length / (float)MAX_FILE_SIZE);

        // if we need to shrink the virtual file
        if (new_files < (int)Names.size()) {

            // if length == 0 then new_files = 0. keep first file and truncate to 0
            if (new_files == 0) {
                new_files = 1;
            }
            
            // delete from disk all files except the first 'new_files',
            // remove their entries from vectors Filedescs and Names
            assert(Filedescs.size() == Names.size());
            for (int i = new_files; i < (int)Names.size(); i++) {
                if (remove(Names[i]) == -1) {
                    printf("[ERROR] %s(): remove('%s')\n", __FUNCTION__, Names[i]);
                    perror("");
                    exit(EXIT_FAILURE);
                }
            }

            Names.erase(Names.begin() + new_files, Names.end());
            Filedescs.erase(Filedescs.begin() + new_files, Filedescs.end());
        }
        // if we need to extend the virtual file
        else if (new_files > (int)Names.size()) {

            // create 'new_files - Names.size()' new files.
            num = Names.size();
            for (int i = 0; i < (int)(new_files - num); i++) {
                fs_open_unique();
            }

            // set the size of the first 'new_files - 1' files to MAX_FILE_SIZE.
            assert((int)Names.size() == new_files);
            for (int i = 0; i < new_files - 1; i++) {
                if (ftruncate64(Filedescs[i], MAX_FILE_SIZE) == (off_t)-1) {
                    printf("[ERROR] %s(): ftruncate64('%s', %Ld) \n", __FUNCTION__, Names[i], MAX_FILE_SIZE);
                    perror("");
                    exit(EXIT_FAILURE);
                }
            }
        }

        Cur = new_files - 1;

        if (length && length % MAX_FILE_SIZE != 0) {
            length = length % MAX_FILE_SIZE;
        } else if (length ) {
            length = MAX_FILE_SIZE;
        } 
        
        if (ftruncate64(Filedescs[Cur], length) == (off_t)-1) {
            printf("[ERROR] %s(): ftruncate64('%s', %Ld) \n", __FUNCTION__, Names[Cur], length);
            perror("");
            exit(EXIT_FAILURE);
        }
    }

    // NOTE: truncate system call does not change file offset. our 
    // implementation sets offset to file size after truncating
    fs_seek(fs_size(), SEEK_SET);
}

/*-------------------------------------------------------
 *                      fs_delete
 *-------------------------------------------------------*/
void VFile::fs_delete(void) // TODO: rename this function to fs_rm (public) and create a protected fs_unlink() that will be called also from fs_truncate.
{
    assert(Cur != -1);
    
    if (Sim->get_simulation_mode() == Simulator::SIMMODE_SIMULATE_IO) {
        
    } else {
        assert(Names.size() == Filedescs.size());
        for (int i = 0; i < (int)Names.size(); i++) {
            if (remove(Names[i]) == -1) {
                printf("[ERROR] %s(): remove('%s')\n", __FUNCTION__, Names[i]);
                perror("");
                exit(EXIT_FAILURE);
            }
        }
    }
}

/*-------------------------------------------------------
 *                         fs_name
 *-------------------------------------------------------*/
char *VFile::fs_name(void)
{
    assert(Cur != -1);
    return Names[Cur];
}

/*-------------------------------------------------------
 *                        fs_size
 *-------------------------------------------------------*/
uint64_t VFile::fs_size(void)
{
    assert(Cur != -1);
    
    if (Sim->get_simulation_mode() == Simulator::SIMMODE_SIMULATE_IO) {
        return Size;
    } else {
        struct stat lastfilestatus;
        int num_files;

        for (int i = 0; i < (int)Names.size() - 1; i++) {
            stat(Names[i], &lastfilestatus);
            assert(lastfilestatus.st_size == MAX_FILE_SIZE);
        }
        
        // get size of last file
        stat(Names[Names.size() - 1], &lastfilestatus);
        
        // compute and return size of virtual file
        num_files = Names.size();
        
        return (num_files - 1) * MAX_FILE_SIZE + lastfilestatus.st_size;
    }
}

/*-------------------------------------------------------
 *                         fs_sync
 *-------------------------------------------------------*/
void VFile::fs_sync(void)
{
    assert(Cur != -1);

    if (Sim->get_simulation_mode() == Simulator::SIMMODE_SIMULATE_IO) {
        
    } else {
        fsync(Filedescs[Cur]);
    }
}

/*-------------------------------------------------------
 *                        fs_append
 *-------------------------------------------------------*/
uint64_t VFile::fs_append(VFile *fsrc, void *buf, size_t bufsize)
{
    size_t count;

    assert(Cur != -1);
    
    fsrc->fs_rewind();
    while ((count = fsrc->fs_read(buf, bufsize))) {
        fs_write(buf, count);
    }

    return fsrc->fs_size();
}

/*-------------------------------------------------------
 *                      cur_fs_read
 *-------------------------------------------------------*/
ssize_t VFile::cur_fs_read(void *buf, size_t count)
{
    ssize_t actually_read;

    assert(Sim->get_simulation_mode() == Simulator::SIMMODE_REAL_IO);
    assert(Cur != -1);

    if (fs_tell() == (int)fs_size()) {
        return 0; // no bytes lef to read
    }
    
    // if we reached end of current file, check if there's a next file in
    // Filedescs vector. if yes, advance to it. if no, error (there should be 
    // available bytes to read).
    if (cur_fs_tell() == (int)cur_fs_size()) {
        if (cur_fs_size() == MAX_FILE_SIZE && Cur < (int)Filedescs.size() - 1) {
            Cur++;
            cur_fs_rewind();
        } else {
            assert(0); // should not get here
        }
    }

    // check if we're going to cross max file size limit: if yes, we're going
    // to write as many bytes as we can in current file and return the
    // number of bytes written. caller must call again fs_write() to
    // write the remaining bytes.
    if (cur_fs_tell() + count > MAX_FILE_SIZE) {
        count = MAX_FILE_SIZE - cur_fs_tell();
    }
    
    if ((actually_read = read(Filedescs[Cur], buf, count)) == -1) {
        printf("[ERROR] %s(): read('%s', %d)\n", __FUNCTION__, Names[Cur], count);
        perror("");
        exit(EXIT_FAILURE);
    }

    assert(actually_read);
    return actually_read;
}

/*-------------------------------------------------------
 *                      cur_fs_write
 *-------------------------------------------------------*/
ssize_t VFile::cur_fs_write(const void *buf, size_t count)
{
    ssize_t actually_written;

    assert(Sim->get_simulation_mode() == Simulator::SIMMODE_REAL_IO);
    assert(Cur != -1);

    // if we reached end of current file, check if there's a next file in
    // Filedescs vector. if yes, advance to it. if no, create a new file.
    if (cur_fs_tell() == MAX_FILE_SIZE) {
        if (Cur == (int)Filedescs.size() - 1) {
            fs_open_unique();
        } else {
            Cur++;
            cur_fs_rewind();
        }
    }

    // (similar to cur_fs_read)
    if (cur_fs_tell() + count > MAX_FILE_SIZE) {
        count = MAX_FILE_SIZE - cur_fs_tell();
    }

    if ((actually_written = write(Filedescs[Cur], buf, count)) == -1) {
        printf("[ERROR] %s(): write('%s', %d)\n", __FUNCTION__, Names[Cur], count);
        perror("");
        exit(EXIT_FAILURE);
    }

    return actually_written;
}

/*-------------------------------------------------------
 *                     cur_fs_size
 *-------------------------------------------------------*/
uint64_t VFile::cur_fs_size(void)
{
    struct stat filestatus;

    assert(Sim->get_simulation_mode() == Simulator::SIMMODE_REAL_IO);
    assert(Cur != -1);

    stat(Names[Cur], &filestatus);
    return filestatus.st_size;
}

/*-------------------------------------------------------
 *                     cur_fs_seek
 *-------------------------------------------------------*/
off_t VFile::cur_fs_seek(off_t offs, int whence)
{
    off_t newoffs;

    assert(Sim->get_simulation_mode() == Simulator::SIMMODE_REAL_IO);
    assert(Cur != -1);
    
    if ((newoffs = lseek(Filedescs[Cur], offs, whence)) == (off_t)-1) {
        printf("[ERROR] %s(): lseek('%s', %Ld, %d)\n", __FUNCTION__, Names[Cur], offs, whence);
        perror("");
        exit(EXIT_FAILURE);
    }

    return newoffs;
}

/*-------------------------------------------------------
 *                      cur_fs_tell
 *-------------------------------------------------------*/
off_t VFile::cur_fs_tell(void)
{
    assert(Sim->get_simulation_mode() == Simulator::SIMMODE_REAL_IO);
    assert(Cur != -1);
    
    return cur_fs_seek(0, SEEK_CUR);
}

/*-------------------------------------------------------
 *                      cur_fs_tell
 *-------------------------------------------------------*/
off_t VFile::cur_fs_rewind(void)
{
    assert(Cur != -1);
    return cur_fs_seek(0, SEEK_SET);
}


/*-------------------------------------------------------
 *                      start_timing
 *-------------------------------------------------------*/
void VFile::start_timing(void)
{
    gettimeofday(&starttime, NULL);
}

/*-------------------------------------------------------
 *                      end_timing
 *-------------------------------------------------------*/
void VFile::end_timing(void)
{
    time_t sec;
    suseconds_t usec;

    gettimeofday(&endtime, NULL);
    sec = endtime.tv_sec - starttime.tv_sec;
    usec = endtime.tv_usec - starttime.tv_usec;
    if (usec < 0) {
        sec -= 1;
        usec += 1000000;
    }
    Sim->inc_time_passed(sec, usec);
}

/*-------------------------------------------------------
 *                      sanity_check
 *-------------------------------------------------------*/
void VFile::sanity_check()
{
#if DBGLVL < 2
    return;
#endif
}
