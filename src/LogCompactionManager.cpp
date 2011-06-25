#include "Global.h"
#include "LogCompactionManager.h"

#include "MemStore.h"
#include "DiskStore.h"

/*========================================================================
 *                           LogCompactionManager
 *========================================================================*/
LogCompactionManager::LogCompactionManager(MemStore *memstore, DiskStore *diskstore)
: GeomCompactionManager(memstore, diskstore)
{
    GeomCompactionManager::set_R(2);
}

/*========================================================================
 *                          ~LogCompactionManager
 *========================================================================*/
LogCompactionManager::~LogCompactionManager()
{

}
