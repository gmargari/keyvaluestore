#ifndef RANGE_H
#define RANGE_H

#include <stdint.h>
#include <cstdlib>
#include <cstring>

#define NO_DISK_BLOCK (-1)

class Range {

public:

    /**
     * constructor
     */
    Range();

    /**
     * destructor
     */
    ~Range();

    /**
     * compare ranges by field 'memsize' (descending order)
     */
    static bool cmp_by_memsize(const Range *r1, const Range *r2);

    /**
     * compare ranges by field 'disksize' (descending order)
     */
    static bool cmp_by_disksize(const Range *r1, const Range *r2);

    /**
     * compare ranges lexicographically by field 'first' (ascending order)
     */
    static bool cmp_by_term(const Range *r1, const Range *r2);

    /**
     * compare ranges by field 'm_block_num' (ascending order)
     */
    static bool cmp_by_block_num(const Range *r1, const Range *r2);

    // Undefined methods (just remove Weffc++ warning)
    Range(const Range&);
    Range& operator=(const Range&);

    const char *m_first;              // terms stored on disk block belong to
    uint32_t    m_firstlen;           // range [m_first, m_last)
    const char *m_last;
    uint32_t    m_lastlen;
    uint64_t    m_memsize;            // byte size of memory tuples
    uint64_t    m_memsize_serialized; // byte size of serialized memory tuples
    uint64_t    m_disksize;           // size of tuples stored on disk block
    int         m_block_num;          // index in DiskStore's vector of disk
                                      // files (we store each block in a
                                      // separate disk file)
};

/*============================================================================
 *                                  Range
 *============================================================================*/
Range::Range()
    : m_first(NULL), m_firstlen(0), m_last(NULL), m_lastlen(0), m_memsize(0),
      m_memsize_serialized(0), m_disksize(0), m_block_num(NO_DISK_BLOCK) {

}

/*============================================================================
 *                                  Range
 *============================================================================*/
Range::~Range() {

}

/*============================================================================
 *                              cmp_by_memsize
 *============================================================================*/
inline bool Range::cmp_by_memsize(const Range *r1, const Range *r2) {
    return (r1->m_memsize > r2->m_memsize); // descending order
}

/*============================================================================
 *                              cmp_by_disksize
 *============================================================================*/
inline bool Range::cmp_by_disksize(const Range *r1, const Range *r2) {
    return (r1->m_disksize > r2->m_disksize); // descending order
}

/*============================================================================
 *                               cmp_by_term
 *============================================================================*/
inline bool Range::cmp_by_term(const Range *r1, const Range *r2) {
    return (strcmp(r1->m_first, r2->m_first) < 0); // ascending order
}


/*============================================================================
 *                            cmp_by_block_num
 *============================================================================*/
inline bool Range::cmp_by_block_num(const Range *r1, const Range *r2) {
    return (r1->m_block_num < r2->m_block_num); // ascending order
}

#endif
