#ifndef RANGE_H
#define RANGE_H

#include <stdint.h>

#define NO_DISK_FILE   (-1)

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
    static bool cmp_by_memsize(const Range& r1, const Range& r2);

    /**
     * compare ranges by field 'disksize' (descending order)
     */
    static bool cmp_by_disksize(const Range& r1, const Range& r2);

    /**
     * compare ranges lexicographically by field 'first' (ascending order)
     */
    static bool cmp_by_term(const Range& r1, const Range& r2);

    /**
     * compare ranges lexicographically by field 'idx' (ascending order)
     */
    static bool cmp_by_file_index(const Range& r1, const Range& r2);

    const char   *m_first;              // terms stored on disk block belong to
    const char   *m_last;               // range [m_first, m_last)
    uint64_t      m_memsize;            // total byte size of memory tuples
    uint64_t      m_memsize_serialized; // total byte size of serialized memory tuples
    uint64_t      m_disksize;           // size of tuples stored on disk block
    int           m_idx;                // index in DiskStore's vector of disk block
};

#endif
