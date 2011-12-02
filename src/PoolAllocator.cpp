// code taken from proteus source code (objstack.c)

#include "PoolAllocator.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

/*============================================================================
 *                                PoolAllocator
 *============================================================================*/
PoolAllocator::PoolAllocator()
    : m_cur_objstack()
{
    init();
}

/*============================================================================
 *                               ~PoolAllocator
 *============================================================================*/
PoolAllocator::~PoolAllocator()
{
    freemem();
}

/*============================================================================
 *                                   init
 *============================================================================*/
void PoolAllocator::init()
{
    m_cur_objstack = new_objstack(MEMCHUNK_SIZE);
}

/*============================================================================
 *                                   alloc
 *============================================================================*/
char *PoolAllocator::alloc(size_t size)
{
    struct objstack *node = NULL;
    size_t newsize;
    void *mem;

    assert(m_cur_objstack != NULL);
    assert(m_cur_objstack->mem != NULL);
    assert(size > 0);

    /* if there is not enough free space in objstack */
    if (m_cur_objstack->len + (int)size > m_cur_objstack->capacity) {

        /* create a new objstack */
        newsize = MEMCHUNK_SIZE;
        assert(size <= newsize);
        if ((node = new_objstack(newsize)) == NULL) {
            return NULL;
        }

        /* chain to old objstack */
        node->prev = m_cur_objstack;

        /* new objstack is the head objstack */
        m_cur_objstack = node;
    }

    mem = (char *)m_cur_objstack->mem + m_cur_objstack->len;
    m_cur_objstack->len += size;

    return (char *)mem;
}

/*============================================================================
 *                                   freemem
 *============================================================================*/
void PoolAllocator::freemem()
{
    struct objstack *tmp, *prev;

    assert(m_cur_objstack != NULL);
    tmp = m_cur_objstack;
    while (tmp) {
        prev = tmp->prev;
        delete(&tmp);
        tmp = prev;
    }
    m_cur_objstack = NULL;
}

/*============================================================================
 *                                  capacity
 *============================================================================*/
int PoolAllocator::capacity()
{
    assert(m_cur_objstack != NULL);
    return m_cur_objstack->capacity;
}

/*============================================================================
 *                                 bytes_used
 *============================================================================*/
int PoolAllocator::bytes_used()
{
    assert(m_cur_objstack != NULL);
    return m_cur_objstack->len;
}

/*============================================================================
 *                                 bytes_free
 *============================================================================*/
int PoolAllocator::bytes_free()
{
    assert(m_cur_objstack != NULL);
    return (m_cur_objstack->capacity - m_cur_objstack->len);
}

/*============================================================================
 *                                new_objstack
 *============================================================================*/
struct PoolAllocator::objstack *PoolAllocator::new_objstack(size_t size)
{
    struct objstack *newobj;

    newobj = (struct objstack *)malloc(sizeof(struct objstack));
    assert(newobj);
    newobj->mem = malloc(size);
    assert(newobj->mem);
    newobj->capacity = size;
    newobj->len = 0;
    newobj->prev = NULL;

    return newobj;
}

/*============================================================================
 *                               delete_objstack
 *============================================================================*/
void PoolAllocator::delete_objstack(struct objstack **node)
{
    free((*node)->mem);
    free(*node);
    node = NULL;
}

