#ifndef DISKSTORE_H
#define DISKSTORE_H

#include "Global.h"
#include "KVTDiskFile.h"

#include <stdint.h>
#include <vector>

using std::vector;

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
     * @param key (in) key to be searched
     * @param value (out) value corresponding to the searched key
     * @param timestamp (out) timestamp of insertion
     * @return true if key was found, false if not
     */
    bool get(const char *key, const char **value, uint64_t *timestamp);

    /**
     * get the value for a specific <key, timestamp> pair
     *
     * @param key (in) key to be searched
     * @param timestamp (in) timestamp of insertion
     * @param value (out) value corresponding to the searched <key, timestamp>
     * @return true if <key, timestamp> was found, false if not
     */
    bool get(const char *key, uint64_t timestamp, const char **value);

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

    void sanity_check();

    vector<KVTDiskFile *> m_diskfiles;
};

#endif
