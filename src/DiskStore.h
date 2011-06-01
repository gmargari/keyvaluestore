#ifndef DISKSTORE_H
#define DISKSTORE_H

#include "Global.h"
#include "KVDiskFile.h"

#include <stdint.h>
#include <vector>

class DiskStore {

friend class CompactionManager;

public:

    /**
     * constructor
     */
    DiskStore();

    /**
     * destructor
     */
    ~DiskStore();

    /**
     * get the value for a specific key
     *
     * @param key key to be searched
     * @return pointer to value of specified key, NULL if key does not exist in
     * map (return value is not copied, caller must copy it)
     */
    char *get(char *key);

    /**
     * number of <key, value> pairs in diskstore
     *
     * @return number of <key, value> pairs in diskstore
     */
    uint64_t num_keys();

    /**
     * byte size of diskstore
     *
     * @return byte size of <key, value> pairs in diskstore
     */
    uint64_t size();

protected:

    std::vector<KVDiskFile *> m_diskfiles;
};

#endif
