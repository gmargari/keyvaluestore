#include "Global.h"
#include "CompactionManager.h"
#include "KVMapInputStream.h"
#include "KVDiskFile.h"
#include "KVDiskFileInputStream.h"
#include "KVDiskFileOutputStream.h"
#include "KVPriorityInputStream.h"

#include <cstdio>

/*========================================================================
 *                           CompactionManager
 *========================================================================*/
CompactionManager::CompactionManager(MemStore *memstore, DiskStore *diskstore)
{
    m_memstore = memstore;
    m_diskstore = diskstore;
}

/*========================================================================
 *                          ~CompactionManager
 *========================================================================*/
CompactionManager::~CompactionManager()
{

}

/*========================================================================
 *                            flush_memstore
 *========================================================================*/
void CompactionManager::flush_memstore(void)
{
    KVDiskFile *kvdiskfile;
    KVMapInputStream *istream;
    KVDiskFileOutputStream *ostream;
    vector<KVDiskFile *>::iterator iter;
    vector<KVInputStream *> istreams;

    /*
     * first, flush memstore to a new file on disk
     */

    // create an input stream for memstore, using it write memstore to
    // a new file on disk and then clear memstore
    kvdiskfile = new KVDiskFile;
    kvdiskfile->open_unique();
    istream = new KVMapInputStream(m_memstore->m_kvmap);
    ostream = new KVDiskFileOutputStream(kvdiskfile);
    copy_stream(istream, ostream);
    m_diskstore->m_diskfiles.push_back(kvdiskfile);
    m_memstore->clear();

    delete istream;
    delete ostream;

    /*
     * if we need to merge some disk files, merge them
     */
    if (m_diskstore->m_diskfiles.size() > 3) {

        // create vector of all input streams that will be merged
        for (iter = m_diskstore->m_diskfiles.begin(); iter != m_diskstore->m_diskfiles.end(); iter++) {
            istreams.push_back(new KVDiskFileInputStream((*iter)));
        }

        // merge input streams, writing output to a new file
        kvdiskfile = new KVDiskFile;
        kvdiskfile->open_unique();
        ostream = new KVDiskFileOutputStream(kvdiskfile);
        merge_streams(istreams, ostream);

        // delete all files merged
        for (int i = 0; i < (int)m_diskstore->m_diskfiles.size(); i++) {
            m_diskstore->m_diskfiles[i]->delete_from_disk();
            delete m_diskstore->m_diskfiles[i];
        }
        m_diskstore->m_diskfiles.clear();

        // add new file to (currently empty) set of disk files
        m_diskstore->m_diskfiles.push_back(kvdiskfile);

        // free memory
        for (int i = 0; i < (int)istreams.size(); i++)
            delete istreams[i];
        delete ostream;
    }
}

/*========================================================================
 *                             copy_stream
 *========================================================================*/
void CompactionManager::copy_stream(KVInputStream *istream, KVOutputStream *ostream)
{
    const char *key, *value;

    while (istream->read(&key, &value)) {
        ostream->write(key, value);
    }
    ostream->flush();
}

/*========================================================================
 *                             merge_streams
 *========================================================================*/
void CompactionManager::merge_streams(vector<KVInputStream *> istreams, KVOutputStream *ostream)
{
    KVPriorityInputStream *istream_heap;

    istream_heap = new KVPriorityInputStream(istreams);
    copy_stream(istream_heap, ostream);
    delete istream_heap;
}

/*=======================================================================*
 *                              sanity_check
 *=======================================================================*/
void CompactionManager::sanity_check()
{
    return_if_dbglvl_lt_2();
}
