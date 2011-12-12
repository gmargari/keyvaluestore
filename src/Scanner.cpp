#include "Global.h"
#include "Scanner.h"

#include "DiskStore.h"
#include "KeyValueStore.h"
#include "DiskFileInputStream.h"
#include "PriorityInputStream.h"

/*============================================================================
 *                                 Scanner
 *============================================================================*/
Scanner::Scanner(KeyValueStore *kvstore)
    : m_kvstore(kvstore) {

}

/*============================================================================
 *                                ~Scanner
 *============================================================================*/
Scanner::~Scanner() {

}

/*============================================================================
 *                                point_get
 *============================================================================*/
int Scanner::point_get(const char *key, uint32_t keylen) {
    const char *k, *v;
    uint32_t klen, vlen, valuelen;
    char *value;
    uint64_t t;
    DiskFileInputStream *disk_istream = NULL;

//    // NOTE: don't read from memstore, it's not thread safe for concurrent puts and gets
//     // first check memstore, since it has the most recent values
//     if (m_kvstore->m_memstore->get(key, keylen, &value, &valuelen, &t)) {
//         free(value); // it has been copied
//         return 1;
//     }

    // search disk files in order, from most recently created to oldest.
    // return the first value found, since this is the most recent value
    pthread_rwlock_rdlock(&m_kvstore->m_diskstore->m_rwlock);
    for (int i = 0; i < (int)m_kvstore->m_diskstore->m_disk_files.size(); i++) {
        disk_istream = new DiskFileInputStream(m_kvstore->m_diskstore->m_disk_files[i], MAX_INDEX_DIST);
        disk_istream->set_key_range(key, keylen, key, keylen, true, true);
        if (disk_istream->read(&k, &klen, &v, &vlen, &t)) {
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
int Scanner::range_get(const char *start_key, uint32_t start_keylen, const char *end_key, uint32_t end_keylen) {
    return range_get(start_key, start_keylen, end_key, end_keylen, true, false);
}

/*============================================================================
 *                                range_get
 *============================================================================*/
int Scanner::range_get(const char *start_key, uint32_t start_keylen, const char *end_key, uint32_t end_keylen, bool start_incl, bool end_incl) {
    const char *k, *v;
    uint32_t klen, vlen;
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
    pistream->set_key_range(start_key, start_keylen, end_key, end_keylen, start_incl, end_incl);
    while (pistream->read(&k, &klen, &v, &vlen, &t)) {
        numkeys++;
    }
    pthread_rwlock_unlock(&m_kvstore->m_diskstore->m_rwlock);

    for (int i = 0; i < (int)istreams.size(); i++) {
        delete istreams[i];
    }

    delete pistream;

    return numkeys;
}
