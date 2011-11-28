#ifndef REQUESTTHROTTLE_H
#define REQUESTTHROTTLE_H

#include <sys/time.h>

class RequestThrottle {

public:

    /**
     * constructor ('target_rate' in number of requests per second)
     */
    RequestThrottle(int target_rate);

    /**
     * destructor
     */
    ~RequestThrottle();

    /**
     * call this before each request call, to achieve 'm_req_per_sec'
     * request rate.
     */
    void throttle();

    // Undefined methods (just remove Weffc++ warning)
    RequestThrottle(const RequestThrottle&);
    RequestThrottle& operator=(const RequestThrottle&);

private:
    int             m_req_per_sec; // target request rate (requests per second)
    struct timeval *m_buffer;      // ring buffer of entries
    int             m_capacity;    // capacity of ring buffer
    int             m_size;        // number of entries in buffer
    int             m_head;        // where next entry will be inserted
};

#endif
