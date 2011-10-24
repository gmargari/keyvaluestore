#include "Global.h"
#include "RangeScanner.h"

#include "MemStore.h"
#include "DiskStore.h"
#include "KeyValueStore.h"
#include "MapInputStream.h"
#include "DiskFileInputStream.h"
#include "PriorityInputStream.h"

#include <cstdlib>

/*============================================================================
 *                               RangeScanner
 *============================================================================*/
RangeScanner::RangeScanner(KeyValueStore *kvstore)
    : m_kvstore(kvstore)
{

}

/*============================================================================
 *                              ~RangeScanner
 *============================================================================*/
RangeScanner::~RangeScanner()
{

}

/*============================================================================
 *                                range_get
 *============================================================================*/
int RangeScanner::point_get(const char *key)
{
    const char *k, *v;
    char *value;
    uint64_t t;
    DiskFileInputStream *disk_istream = NULL;

    // first check memstore, since it has the most recent values
    if (m_kvstore->m_memstore->get(key, &value, &t)) {
        free(value); // it has been copied
        return 1;
    }

    // search disk files in order, from most recently created to oldest.
    // return the first value found, since this is the most recent value
    for (int i = 0; i < (int)m_kvstore->m_diskstore->m_disk_files.size(); i++) {
        disk_istream = new DiskFileInputStream(m_kvstore->m_diskstore->m_disk_files[i], MAX_INDEX_DIST);
        disk_istream->set_key_range(key, key, true, true);
        if (disk_istream->read(&k, &v, &t)) {
            delete disk_istream;
            return 1;
        }
    }

    if (disk_istream) {
        delete disk_istream;
    }

    return 0;
}

/*============================================================================
 *                                range_get
 *============================================================================*/
int RangeScanner::range_get(const char *start_key, const char *end_key)
{
    return range_get(start_key, end_key, true, false);
}

/*============================================================================
 *                                range_get
 *============================================================================*/
int RangeScanner::range_get(const char *start_key, const char *end_key, bool start_incl, bool end_incl)
{
    const char *k, *v;
    uint64_t t;
    int numkeys = 0;
    PriorityInputStream *pistream;
    vector<InputStream *> istreams;

    // create a priority stream, containing a stream for memstore and one
    // stream for each disk file
    for (int i = 0; i < (int)m_kvstore->m_diskstore->m_disk_files.size(); i++) {
        istreams.push_back(new DiskFileInputStream(m_kvstore->m_diskstore->m_disk_files[i], MAX_INDEX_DIST));
    }
    istreams.push_back(m_kvstore->m_memstore->m_inputstream);
    pistream = new PriorityInputStream(istreams);

    // get all keys between 'start_key' and 'end_key'
    pistream->set_key_range(start_key, end_key, start_incl, end_incl);
    while (pistream->read(&k, &v, &t)) {
        numkeys++;
    }

    for (int i = 0; i < (int)m_kvstore->m_diskstore->m_disk_files.size(); i++) {
        delete istreams[i];
    }

    delete pistream;

    return numkeys;
}
