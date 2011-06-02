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
    KVDiskFile *kvdiskfile = new KVDiskFile;
    KVMapInputStream iscanner(&(m_memstore->m_kvmap));
    KVDiskFileOutputStream oscanner(kvdiskfile);
    
    printf("========== Flush mem to [%s] ==========\n", kvdiskfile->name());
    while (iscanner.read(&key, &value)) {
        oscanner.write(key, value);
        printf("[%s] [%s]\n", key, value);
    }
    oscanner.flush();
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
    KVDiskFile *kvdiskfile = new KVDiskFile;
    KVDiskFileOutputStream oscanner(kvdiskfile);

    printf("========== Merge files to [%s] ==========\n", kvdiskfile->name());
    for (iter = m_diskstore->m_diskfiles.begin(); iter != m_diskstore->m_diskfiles.end(); iter++) {
        KVDiskFileInputStream scanner((*iter));
        while (scanner.read(&key, &value)) {
            oscanner.write(key, value);
            printf("[%s] [%s]\n", key, value);
        }
    }
    oscanner.flush();

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

    for (iter = m_diskstore->m_diskfiles.begin(); iter != m_diskstore->m_diskfiles.end(); iter++) {
        printf("========== Cat file [%s] ==========\n", (*iter)->name());
        KVDiskFileInputStream scanner((*iter));
        while (scanner.read(&key, &value)) {
            printf("[%s] [%s]\n", key, value);
        }
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
