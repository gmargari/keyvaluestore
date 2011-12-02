// code taken from proteus source code (pnodestack.h)

#ifndef POOLALLOCATOR_H
#define POOLALLOCATOR_H

#define MEMCHUNK_SIZE 65536 // 64KB chunks

#include <stdlib.h>

class PoolAllocator {

public:

    /**
     * constructor
     */
    PoolAllocator();

    /**
     * destructor
     */
    ~PoolAllocator();

    /* initialize pool (should be called after free()) */
    void init();

    /* allocate 'size' bytes from pool */
    char *alloc(size_t size);

    /* free all memory used by pool */
    void freemem();

    /* return capacity of pool */
    int capacity();

    /* return number of used bytes in pool */
    int bytes_used();

    /* return number of free bytes in pool */
    int bytes_free();

    // Undefined methods (just remove Weffc++ warning)
    PoolAllocator(const PoolAllocator&);
    PoolAllocator& operator=(const PoolAllocator&);

private:

    /* a stack from which we can allocate objects */
    struct objstack {
        void *mem;               /* memory to allocate objects from */
        int capacity;            /* capacity of allocated memory */
        int len;                 /* bytes used so far */
        struct objstack *prev;   /* previously allocated objstack */
    };

    /* create new objstack */
    struct objstack *new_objstack(size_t size);

    /* delete an objstack */
    void delete_objstack(struct objstack **node);

    struct objstack *m_cur_objstack;
};

#endif
