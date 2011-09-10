#include "Global.h"
#include "LogCompactionManager.h"

/*============================================================================
 *                             LogCompactionManager
 *============================================================================*/
LogCompactionManager::LogCompactionManager(MemStore *memstore, DiskStore *diskstore)
    : GeomCompactionManager(memstore, diskstore)
{
    GeomCompactionManager::set_R(2);
}

/*============================================================================
 *                            ~LogCompactionManager
 *============================================================================*/
LogCompactionManager::~LogCompactionManager()
{

}
