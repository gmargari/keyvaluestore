#include "Range.h"

#include <cstdlib>
#include <cstring>

/*============================================================================
 *                                  Range
 *============================================================================*/
Range::Range()
    : m_first(NULL), m_last(NULL), m_memsize(0), m_memsize_serialized(0),
      m_disksize(0), m_block_num(NO_DISK_BLOCK)
{

}

/*============================================================================
 *                                 ~Range
 *============================================================================*/
Range::~Range()
{

}

/*============================================================================
 *                              cmp_by_memsize
 *============================================================================*/
bool Range::cmp_by_memsize(const Range *r1, const Range *r2)
{
    return (r1->m_memsize > r2->m_memsize); // descending order
}

/*============================================================================
 *                              cmp_by_disksize
 *============================================================================*/
bool Range::cmp_by_disksize(const Range *r1, const Range *r2)
{
    return (r1->m_disksize > r2->m_disksize); // descending order
}

/*============================================================================
 *                               cmp_by_term
 *============================================================================*/
bool Range::cmp_by_term(const Range *r1, const Range *r2)
{
    return (strcmp(r1->m_first, r2->m_first) < 0); // ascending order
}


/*============================================================================
 *                            cmp_by_block_num
 *============================================================================*/
bool Range::cmp_by_block_num(const Range *r1, const Range *r2)
{
    return (r1->m_block_num < r2->m_block_num); // ascending order
}
