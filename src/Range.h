#ifndef RANGE_H
#define RANGE_H

#include <stdint.h>

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
    const char *m_last;               // range [m_first, m_last)
    uint64_t    m_memsize;            // byte size of memory tuples
    uint64_t    m_memsize_serialized; // byte size of serialized memory tuples
    uint64_t    m_disksize;           // size of tuples stored on disk block
    int         m_block_num;          // index in DiskStore's vector of disk
                                      // files (we store each block in a
                                      // separate disk file)
};

#endif
