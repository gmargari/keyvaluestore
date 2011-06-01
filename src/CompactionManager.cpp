#include "Global.h"
#include "CompactionManager.h"
#include "KVDiskFile.h"
#include "KVMapInputStream.h"
#include "KVDiskFileInputStream.h"

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

//TEST
/*========================================================================
 *                            flush_memstore
 *========================================================================*/
void CompactionManager::flush_memstore(void)
{
    KVDiskFile *diskfile;
    KVMapInputStream scanner(&(m_memstore->m_kvmap));
    const char *key, *value;

    printf("-----------[ compaction ]----------\n");

    diskfile = new KVDiskFile;
    printf(">>> %s\n", diskfile->name());
    while (scanner.read(&key, &value)) {
        diskfile->write(key, value);
        printf("[%s] [%s]\n", key, value);
    }
    diskfile->sync();
//     diskfile->close(); // if I close it, cannot use it next by read
    m_diskstore->m_diskfiles.push_back(diskfile);
    m_memstore->clear();

    printf("-----------------------------------\n\n");
}

// TEST
/*========================================================================
 *                             catdiskfiles
 *========================================================================*/
void CompactionManager::catdiskfiles()
{
    // TODO:
    // TODO: All these should be done using KVDiskScanner
    // TODO:
    
    std::vector<KVDiskFile *>::iterator iter;
    const char *key, *value;

    printf("-----------[ catdiskfiles ]----------\n");
    
    for (iter = m_diskstore->m_diskfiles.begin(); iter != m_diskstore->m_diskfiles.end(); iter++) {
        printf(">>> File: %s\n", (*iter)->name());
        KVDiskFileInputStream scanner((*iter));
        while (scanner.read(&key, &value)) {
            printf("[%s] [%s]\n", key, value);
        }
        printf("\n");
    }
    printf("-------------------------------------\n\n");
}