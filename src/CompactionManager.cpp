#include "Global.h"
#include "CompactionManager.h"
#include "KVDiskFile.h"
#include "KVMapInputStream.h"
#include "KVDiskFileInputStream.h"
#include "KVDiskFileOutputStream.h"

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
    KVMapInputStream *iscanner;
    KVDiskFileOutputStream *oscanner;
    
    printf("========== Flush mem ==========\n");
    kvdiskfile = new KVDiskFile;
    iscanner = new KVMapInputStream(&(m_memstore->m_kvmap));
    oscanner = new KVDiskFileOutputStream(kvdiskfile);
    while (iscanner->read(&key, &value)) {
        oscanner->write(key, value);
        printf("[%s] [%s]\n", key, value);
    }
    oscanner->flush();
    delete iscanner;
    delete oscanner;
    
//     kvdiskfile->close(); // if I close it, cannot use it next by read
    m_diskstore->m_diskfiles.push_back(kvdiskfile);
    m_memstore->clear();
    
}

// TEST
/*========================================================================
 *                             merge_all_files
 *========================================================================*/
void CompactionManager::merge_all_files()
{
    const char *key, *value;
    std::vector<KVDiskFile *>::iterator iter;
    KVDiskFile *kvdiskfile;
    KVDiskFileOutputStream *oscanner;
    KVDiskFileInputStream *iscanner;

// NOTE: if after merging a value is greater than MAX_KVSIZE --> error!
//     assert(strlen(key) + 1 <= MAX_KVSIZE);
//     if (strlen(key) + 1 > MAX_KVSIZE || strlen(value) + 1> MAX_KVSIZE) {
//         printf("Error: key or value size greater than max size allowed (%ld)\n", MAX_KVSIZE);
//         assert(0);
//         return false;
//     }

    printf("========== Merge files to ==========\n");

    kvdiskfile = new KVDiskFile;
    oscanner = new KVDiskFileOutputStream(kvdiskfile);
    for (iter = m_diskstore->m_diskfiles.begin(); iter != m_diskstore->m_diskfiles.end(); iter++) {
        iscanner = new KVDiskFileInputStream((*iter));
        while (iscanner->read(&key, &value)) {
            oscanner->write(key, value);
            printf("[%s] [%s]\n", key, value);
        }
        delete iscanner;
    }
    oscanner->flush();
    delete oscanner;

    m_diskstore->m_diskfiles.push_back(kvdiskfile);
}

// TEST
/*========================================================================
 *                             catdiskfiles
 *========================================================================*/
void CompactionManager::catdiskfiles()
{
    const char *key, *value;
    std::vector<KVDiskFile *>::iterator iter;
    KVDiskFileInputStream *iscanner;

    for (iter = m_diskstore->m_diskfiles.begin(); iter != m_diskstore->m_diskfiles.end(); iter++) {
        printf("========== Cat file ==========\n");
        iscanner = new KVDiskFileInputStream(*iter);
        while (iscanner->read(&key, &value)) {
            printf("[%s] [%s]\n", key, value);
        }
        delete iscanner;
    }
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
