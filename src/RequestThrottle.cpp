#include "Global.h"
#include "RequestThrottle.h"

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>

/**
 * throttle request per second using a ring buffer:
 * http://stackoverflow.com/questions/1407113/throttling-method-calls-to-m-requests-in-n-seconds
 */

/*============================================================================
 *                               RequestThrottle
 *============================================================================*/
RequestThrottle::RequestThrottle(int target_rate)
    : m_req_per_sec(target_rate), m_buffer(), m_capacity(target_rate), m_size(0),
      m_head(0)
{
    if (m_req_per_sec) {
        m_buffer = new timeval[m_capacity];
        assert(m_buffer);
    }
}

/*============================================================================
 *                               ~RequestThrottle
 *============================================================================*/
RequestThrottle::~RequestThrottle()
{
   delete[] m_buffer;
}

/*============================================================================
 *                                   throttle
 *============================================================================*/
void RequestThrottle::throttle()
{
    struct timeval now;
    double diff_sec;

    if (m_req_per_sec == 0) { // throttling disabled
        return;
    }

    if (m_size == m_capacity) {

        // check the oldest entry. if it's less than 1 second in the past,
        // sleep for the time difference
        gettimeofday(&now, NULL);
        diff_sec = (now.tv_sec - m_buffer[m_head].tv_sec)
                   + (now.tv_usec - m_buffer[m_head].tv_usec)/1000000.0;
        if (diff_sec < 1) {
            usleep((1 - diff_sec) * 1000000);
        }
    }

    gettimeofday(&now, NULL);
    assert(m_size != m_capacity ||
           now.tv_sec - m_buffer[m_head].tv_sec + (now.tv_usec - m_buffer[m_head].tv_usec)/1000000.0 >= 1);

    // add new entry (probably overwriting oldest entry)
    m_buffer[m_head].tv_sec = now.tv_sec;
    m_buffer[m_head].tv_usec = now.tv_usec;
    m_head = (m_head + 1) % m_capacity;
    if (m_size < m_capacity) {
        m_size++;
    }
}
