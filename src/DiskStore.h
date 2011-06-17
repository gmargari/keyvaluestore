#ifndef DISKSTORE_H
#define DISKSTORE_H

#include <stdint.h>
#include <vector>

using std::vector;

class KVTDiskFile;
class KVTDiskFileInputStream;

class DiskStore {

friend class CompactionManager;
friend class ImmCompactionManager;
friend class GeomCompactionManager;
friend class LogCompactionManager;
friend class UrfCompactionManager;

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
     * get the (copy of) value for a specific key.
     *
     * @param key (in) key to be searched
     * @param value (out) value corresponding to the searched key
     * @param timestamp (out) timestamp of insertion
     * @return true if key was found, false if not
     */
    bool get(const char *key, char **value, uint64_t *timestamp);

    /**
     * get the (copy of) value for a specific <key, timestamp> pair.
     *
     * @param key (in) key to be searched
     * @param timestamp (in) timestamp of insertion
     * @param value (out) value corresponding to the searched <key, timestamp>
     * @return true if <key, timestamp> was found, false if not
     */
    bool get(const char *key, uint64_t timestamp, char **value);

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

    int sanity_check();

    vector<KVTDiskFile *>               m_disk_files;
    vector<KVTDiskFileInputStream *>    m_disk_istreams;
};

#endif
