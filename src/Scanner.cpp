#include "Global.h"
#include "Scanner.h"

#include "MemStore.h"
#include "DiskStore.h"
#include "KeyValueStore.h"
#include "MapInputStream.h"
#include "DiskFileInputStream.h"
#include "PriorityInputStream.h"

/*============================================================================
 *                                 Scanner
 *============================================================================*/
Scanner::Scanner(KeyValueStore *kvstore)
    : m_kvstore(kvstore)
{

}

/*============================================================================
 *                                ~Scanner
 *============================================================================*/
Scanner::~Scanner()
{

}

/*============================================================================
 *                                point_get
 *============================================================================*/
int Scanner::point_get(const char *key)
{
    const char *k, *v;
    char *value;
    uint64_t t;
    DiskFileInputStream *disk_istream = NULL;

    // NOTE: don't read from memstore, it's not thread safe for concurrent puts and gets
//     // first check memstore, since it has the most recent values
//     if (m_kvstore->m_memstore->get(key, &value, &t)) {
//         free(value); // it has been copied
//         return 1;
//     }

    // search disk files in order, from most recently created to oldest.
    // return the first value found, since this is the most recent value
    pthread_rwlock_rdlock(&m_kvstore->m_diskstore->m_rwlock);
    for (int i = 0; i < (int)m_kvstore->m_diskstore->m_disk_files.size(); i++) {
        disk_istream = new DiskFileInputStream(m_kvstore->m_diskstore->m_disk_files[i], MAX_INDEX_DIST);
        disk_istream->set_key_range(key, key, true, true);
        if (disk_istream->read(&k, &v, &t)) {
            pthread_rwlock_unlock(&m_kvstore->m_diskstore->m_rwlock);
            delete disk_istream;
            return 1;
        }
        delete disk_istream;
    }
    pthread_rwlock_unlock(&m_kvstore->m_diskstore->m_rwlock);

    return 0;
}

/*============================================================================
 *                                range_get
 *============================================================================*/
int Scanner::range_get(const char *start_key, const char *end_key)
{
    return range_get(start_key, end_key, true, false);
}

/*============================================================================
 *                                range_get
 *============================================================================*/
int Scanner::range_get(const char *start_key, const char *end_key, bool start_incl, bool end_incl)
{
    const char *k, *v;
    uint64_t t;
    int numkeys = 0, diskfiles;
    PriorityInputStream *pistream;
    vector<InputStream *> istreams;

    // create a priority stream, containing a stream for memstore and one
    // stream for each disk file
    pthread_rwlock_rdlock(&m_kvstore->m_diskstore->m_rwlock);
    diskfiles = m_kvstore->m_diskstore->m_disk_files.size();
    for (int i = 0; i < diskfiles; i++) {
        istreams.push_back(new DiskFileInputStream(m_kvstore->m_diskstore->m_disk_files[i], MAX_INDEX_DIST));
    }
    // NOTE: don't read from memstore, it's not thread safe for concurrent puts and gets
//    istreams.push_back(m_kvstore->m_memstore->new_map_inputstream());
    pistream = new PriorityInputStream(istreams);

    // get all keys between 'start_key' and 'end_key'
    pistream->set_key_range(start_key, end_key, start_incl, end_incl);
    while (pistream->read(&k, &v, &t)) {
        numkeys++;
    }
    pthread_rwlock_unlock(&m_kvstore->m_diskstore->m_rwlock);

    for (int i = 0; i < istreams.size(); i++) {
        delete istreams[i];
    }

    delete pistream;

    return numkeys;
}
