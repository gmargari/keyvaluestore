// Copyright (c) 2001 Xiao Zhang
//
// http://alumni.cs.ucr.edu/~ysong/cs160/lab6/reader-writer.html
//
// When a reader arrives and a writer is waiting, the reader is suspended
// behind the writer instead of being admitted immediately. In this way, a
// writer has to wait for readers that were active when it arrived to finish
// but does not to wait for readers that came along after it.
//
// For more details, see Andrew S. Tanenbaum and Albert S. Woodhull,
// Operating Systems: Design and Implementation, 2nd Edition,
// Prentice-Hall, pp. 77-80

#ifndef SRC_READWRITEMUTEX_H_
#define SRC_READWRITEMUTEX_H_

#include <unistd.h>
#include <pthread.h>

class ReadWriteMutex {
  public:
    ReadWriteMutex() : rc(), rc_mutex(), r_mutex(), w_mutex() {
        rc = 0;
        pthread_mutex_init(&rc_mutex, NULL);
        pthread_mutex_init(&r_mutex, NULL);
        pthread_mutex_init(&w_mutex, NULL);
    }

    void read_lock() {
        pthread_mutex_lock(&r_mutex);
        pthread_mutex_lock(&rc_mutex);
        rc++;
        if (rc == 1) pthread_mutex_lock(&w_mutex);
        pthread_mutex_unlock(&rc_mutex);
        pthread_mutex_unlock(&r_mutex);
    }

    void read_unlock() {
        pthread_mutex_lock(&rc_mutex);
        rc--;
        if (rc == 0) pthread_mutex_unlock(&w_mutex);
        pthread_mutex_unlock(&rc_mutex);
    }

    void write_lock() {
        pthread_mutex_lock(&r_mutex);
        pthread_mutex_lock(&w_mutex);
    }

    void write_unlock() {
        pthread_mutex_unlock(&w_mutex);
        pthread_mutex_unlock(&r_mutex);
    }

  private:
    int rc;  // readcount
    pthread_mutex_t rc_mutex;
    pthread_mutex_t r_mutex;
    pthread_mutex_t w_mutex;
};

#endif  // SRC_READWRITEMUTEX_H_
