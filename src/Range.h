// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#ifndef SRC_RANGE_H_
#define SRC_RANGE_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "./Slice.h"

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

    Slice    m_first;               // first key stored on disk block
    uint64_t m_memsize;             // byte size of memory tuples
    uint64_t m_memsize_serialized;  // byte size of serialized memory tuples
    uint64_t m_disksize;            // size of tuples stored on disk block
    int      m_block_num;           // index in DiskStore's vector of disk
                                    // files (we store each block in a
                                    // separate disk file)
};

/*============================================================================
 *                                  Range
 *============================================================================*/
Range::Range()
    : m_first(), m_memsize(0), m_memsize_serialized(0), m_disksize(0),
      m_block_num(NO_DISK_BLOCK) {
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
    return (r1->m_memsize > r2->m_memsize);  // desc order
}

/*============================================================================
 *                              cmp_by_disksize
 *============================================================================*/
inline bool Range::cmp_by_disksize(const Range *r1, const Range *r2) {
    return (r1->m_disksize > r2->m_disksize);  // desc order
}

/*============================================================================
 *                               cmp_by_term
 *============================================================================*/
inline bool Range::cmp_by_term(const Range *r1, const Range *r2) {
    return (strcmp(r1->m_first.data(), r2->m_first.data()) < 0);  // asc order
}


/*============================================================================
 *                            cmp_by_block_num
 *============================================================================*/
inline bool Range::cmp_by_block_num(const Range *r1, const Range *r2) {
    return (r1->m_block_num < r2->m_block_num);  // asc order
}

#endif  // SRC_RANGE_H_
