#include "CompactionManager.h"

#include <cstdio>

#include "KVDiskFile.h"
#include "KVMapScanner.h"
#include "KVDiskFileScanner.h"

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
 *                          memstore_to_diskfile
 *========================================================================*/
void CompactionManager::memstore_to_diskfile(void)
{
//     KVDiskFile *diskfile;
    KVMapScanner scanner(&(m_memstore->m_kvmap));
    const char *key, *value;

//     diskfile = new KVDiskFile;
    while (scanner.next(&key, &value)) {
        printf("[%s] -> [%s]\n", key, value);
//         diskfile->append(key, value); // TODO
    }
//     diskfile->flush();
//     diskfile->close();
//     m_diskstore->m_diskfiles.push_back(diskfile);
}