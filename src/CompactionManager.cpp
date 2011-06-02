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
    const char *key, *value;
    KVDiskFile *kvdiskfile;
    KVMapInputStream *istream;
    KVDiskFileOutputStream *ostream, *merged_ostream;
    vector<KVDiskFile *>::iterator iter;
    vector<KVInputStream *> istreams;
    
    /*
     * first, flush memstore to a new file on disk
     */
    
    printf("========== Flush mem ==========\n");
    // create an input stream for memstore, using it write memstore to 
    // a new file on disk and then clear memstore
    kvdiskfile = new KVDiskFile;
    istream = new KVMapInputStream(&(m_memstore->m_kvmap));
    ostream = new KVDiskFileOutputStream(kvdiskfile);
    while (istream->read(&key, &value)) {
        ostream->write(key, value);
        printf("[%s] [%s]\n", key, value);
    }
    ostream->flush();
    m_diskstore->m_diskfiles.push_back(kvdiskfile);
    m_memstore->clear();
    
    delete istream;
    delete ostream;
    
    /* 
     * if we need to merge some disk files, merge them
     */
    if (m_diskstore->m_diskfiles.size() > 2) {
        
        printf("========== Merge files to ==========\n");

        // create vector of all input streams that will be merged
        for (iter = m_diskstore->m_diskfiles.begin(); iter != m_diskstore->m_diskfiles.end(); iter++) {
            istreams.push_back(new KVDiskFileInputStream((*iter)));
        }

        // merge input streams, writing output to a new file
        kvdiskfile = new KVDiskFile;
        merged_ostream = new KVDiskFileOutputStream(kvdiskfile);
        merge_istreams(istreams, merged_ostream);

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
        delete merged_ostream;
    }
}

/*========================================================================
 *                             merge_istreams
 *========================================================================*/
void CompactionManager::merge_istreams(vector<KVInputStream *> istreams, KVDiskFileOutputStream *ostream)
{
    const char *key, *value;
    vector<KVDiskFile *>::iterator iter;
    KVPriorityInputStream *istream_heap;

    istream_heap = new KVPriorityInputStream(istreams);    
    while (istream_heap->read(&key, &value)) {
        ostream->write(key, value);
        printf("[%s] [%s]\n", key, value);
    }
    ostream->flush();
    delete istream_heap;
}

/*=======================================================================*
 *                              sanity_check
 *=======================================================================*/
void CompactionManager::sanity_check()
{
#if DBGLVL < 2
    return;
#endif
}

// TEST
/*========================================================================
 *                             catdiskfiles
 *========================================================================*/
void CompactionManager::catdiskfiles()
{
    const char *key, *value;
    vector<KVDiskFile *>::iterator iter;
    KVDiskFileInputStream *istream;

    for (iter = m_diskstore->m_diskfiles.begin(); iter != m_diskstore->m_diskfiles.end(); iter++) {
        printf("========== Cat file ==========\n");
        istream = new KVDiskFileInputStream(*iter);
        while (istream->read(&key, &value)) {
            printf("[%s] [%s]\n", key, value);
        }
        delete istream;
    }
}