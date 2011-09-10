#include "Global.h"
#include "RangeScanner.h"

#include "MemStore.h"
#include "DiskStore.h"
#include "KeyValueStore.h"
#include "MapInputStream.h"
#include "DiskFileInputStream.h"
#include "PriorityInputStream.h"

/*============================================================================
 *                               RangeScanner
 *============================================================================*/
RangeScanner::RangeScanner(KeyValueStore *kvstore)
    : m_pistream()
{
    vector<InputStream *> istreams;

    istreams.push_back(kvstore->m_memstore->m_inputstream);
    for (int i = 0; i < (int)kvstore->m_diskstore->m_disk_istreams.size(); i++) {
        istreams.push_back(kvstore->m_diskstore->m_disk_istreams[i]);
    }

    m_pistream = new PriorityInputStream(istreams);
}

/*============================================================================
 *                              ~RangeScanner
 *============================================================================*/
RangeScanner::~RangeScanner()
{
    delete m_pistream;
}

/*============================================================================
 *                              set_key_range
 *============================================================================*/
void RangeScanner::set_key_range(const char *start_key, const char *end_key)
{
    m_pistream->set_key_range(start_key, end_key);
}

/*============================================================================
 *                              set_key_range
 *============================================================================*/
void RangeScanner::set_key_range(const char *start_key, const char *end_key, bool start_incl, bool end_incl)
{
    m_pistream->set_key_range(start_key, end_key, start_incl, end_incl);
}

/*============================================================================
 *                                get_next
 *============================================================================*/
bool RangeScanner::get_next(char **key, char **value, uint64_t *timestamp)
{
    const char *k, *v;

    if (m_pistream->read(&k, &v, timestamp)) {
        *key = strdup(k);
        *value = strdup(v);
        return true;
    } else {
        *key = NULL;
        *value = NULL;
        return false;
    }
}
