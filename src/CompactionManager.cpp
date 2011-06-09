#include "Global.h"
#include "CompactionManager.h"
#include "KVTMapInputStream.h"
#include "KVTDiskFile.h"
#include "KVTDiskFileInputStream.h"
#include "KVTDiskFileOutputStream.h"
#include "KVTPriorityInputStream.h"

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
    KVTDiskFile *kvtdiskfile;
    KVTMapInputStream *istream;
    KVTDiskFileOutputStream *ostream;
    vector<KVTDiskFile *>::iterator iter;
    vector<KVTInputStream *> istreams;

    /*
     * first, flush memstore to a new file on disk
     */

    // create an input stream for memstore, using it write memstore to
    // a new file on disk and then clear memstore
    kvtdiskfile = new KVTDiskFile;
    kvtdiskfile->open_unique();
    istream = new KVTMapInputStream(m_memstore->m_kvtmap);
    ostream = new KVTDiskFileOutputStream(kvtdiskfile);
    copy_stream(istream, ostream);
    m_diskstore->m_diskfiles.push_back(kvtdiskfile);
    m_memstore->clear();

    delete istream;
    delete ostream;

    /*
     * if we need to merge some disk files, merge them
     */
    if (m_diskstore->m_diskfiles.size() > 3) {

        // create vector of all input streams that will be merged
        for (iter = m_diskstore->m_diskfiles.begin(); iter != m_diskstore->m_diskfiles.end(); iter++) {
            istreams.push_back(new KVTDiskFileInputStream((*iter)));
        }

        // merge input streams, writing output to a new file
        kvtdiskfile = new KVTDiskFile;
        kvtdiskfile->open_unique();
        ostream = new KVTDiskFileOutputStream(kvtdiskfile);
        merge_streams(istreams, ostream);

        // delete all files merged
        for (int i = 0; i < (int)m_diskstore->m_diskfiles.size(); i++) {
            m_diskstore->m_diskfiles[i]->delete_from_disk();
            delete m_diskstore->m_diskfiles[i];
        }
        m_diskstore->m_diskfiles.clear();

        // add new file to (currently empty) set of disk files
        m_diskstore->m_diskfiles.push_back(kvtdiskfile);

        // free memory
        for (int i = 0; i < (int)istreams.size(); i++)
            delete istreams[i];
        delete ostream;
    }
}

/*========================================================================
 *                             copy_stream
 *========================================================================*/
void CompactionManager::copy_stream(KVTInputStream *istream, KVTOutputStream *ostream)
{
    const char *key, *value;
    uint64_t timestamp;

    while (istream->read(&key, &value, &timestamp)) {
        ostream->write(key, value, timestamp);
    }
    ostream->flush();
}

/*========================================================================
 *                             merge_streams
 *========================================================================*/
void CompactionManager::merge_streams(vector<KVTInputStream *> istreams, KVTOutputStream *ostream)
{
    KVTPriorityInputStream *istream_heap;

    istream_heap = new KVTPriorityInputStream(istreams);
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
