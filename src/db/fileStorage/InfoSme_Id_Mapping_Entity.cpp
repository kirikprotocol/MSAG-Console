#include "InfoSme_Id_Mapping_Entity.hpp"

smsc::util::RefObjectRegistry<smsc::core::synchronization::RecursiveMutex,
                              InfoSme_Id_Mapping_Entity::SmscId_Key> InfoSme_Id_Mapping_Entity::_mutexRegistry_ForSmscIdExAccess;

smsc::core::synchronization::Mutex InfoSme_Id_Mapping_Entity::_mutexRegistryLock_ForSmscIdExAccess;
