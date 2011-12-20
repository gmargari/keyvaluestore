// Copyright (c) 2011 Giorgos Margaritis. All rights reserved.

#include "./Global.h"
#include "./Scanner.h"

#include <vector>

#include "./DiskStore.h"
#include "./MemStore.h"
#include "./KeyValueStore.h"
#include "./DiskFileInputStream.h"
#include "./PriorityInputStream.h"

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
    Slice k, v;
    uint64_t t;
    DiskFileInputStream *disk_istream = NULL;
    MemStore *memstore = m_kvstore->m_memstore;
    DiskStore *diskstore = m_kvstore->m_diskstore;

    // NOTE: don't read from memstore, it's not thread-safe for concurrent puts
    // and gets
     // first check memstore, since it has the most recent values
//     k = Slice(key, keylen);
//     if (memstore->get(k, &v, &t)) {
//         free(const_cast<char *>(v.data()));  // it has been copied
//         return 1;
//     }

    // search disk files in order, from most recently created to oldest.
    // return the first value found, since this is the most recent value
    diskstore->read_lock();
    for (int i = 0; i < diskstore->get_num_disk_files(); i++) {
        DiskFile *dfile = diskstore->get_diskfile(i);
        disk_istream = new DiskFileInputStream(dfile, MAX_INDEX_DIST);
        disk_istream->set_key_range(key, key, true, true);
        if (disk_istream->read(&k, &v, &t)) {
            diskstore->read_unlock();
            delete disk_istream;
            return 1;
        }
        delete disk_istream;
    }
    diskstore->read_unlock();

    return 0;
}

/*============================================================================
 *                                range_get
 *============================================================================*/
int Scanner::range_get(const char *start_key, uint32_t start_keylen,
                       const char *end_key, uint32_t end_keylen) {
    return range_get(start_key, start_keylen, end_key, end_keylen, true, false);
}

/*============================================================================
 *                                range_get
 *============================================================================*/
int Scanner::range_get(const char *start_key, uint32_t start_keylen,
                       const char *end_key, uint32_t end_keylen,
                       bool start_incl, bool end_incl) {
    Slice startkey, endkey, k, v;
    uint64_t t;
    int numkeys = 0, diskfiles;
    PriorityInputStream *pistream;
    vector<InputStream *> istreams;
    MemStore *memstore = m_kvstore->m_memstore;
    DiskStore *diskstore = m_kvstore->m_diskstore;

    // create a priority stream, containing a stream for memstore and one
    // stream for each disk file
    diskstore->read_lock();
    diskfiles = diskstore->get_num_disk_files();
    for (int i = 0; i < diskfiles; i++) {
        DiskFile *dfile = diskstore->get_diskfile(i);
        istreams.push_back(new DiskFileInputStream(dfile, MAX_INDEX_DIST));
    }
    // NOTE: don't read from memstore, it's not thread safe for concurrent puts
    //       and gets
//    istreams.push_back(memstore->new_map_inputstream());
    pistream = new PriorityInputStream(istreams);

    // get all keys between 'start_key' and 'end_key'
    startkey = Slice(start_key, start_keylen);
    endkey = Slice(end_key, end_keylen);
    pistream->set_key_range(startkey, endkey, start_incl, end_incl);
    while (pistream->read(&k, &v, &t)) {
        numkeys++;
    }
    diskstore->read_unlock();

    for (int i = 0; i < (int)istreams.size(); i++) {
        delete istreams[i];
    }

    delete pistream;

    return numkeys;
}
