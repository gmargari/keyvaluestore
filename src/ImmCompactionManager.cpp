#include "Global.h"
#include "ImmCompactionManager.h"

/*============================================================================
 *                             ImmCompactionManager
 *============================================================================*/
ImmCompactionManager::ImmCompactionManager(MemStore *memstore, DiskStore *diskstore)
: GeomCompactionManager(memstore, diskstore)
{
    GeomCompactionManager::set_P(1);
}

/*============================================================================
 *                            ~ImmCompactionManager
 *============================================================================*/
ImmCompactionManager::~ImmCompactionManager()
{

}
