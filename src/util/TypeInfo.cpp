#include "TypeInfo.h"

namespace smsc {
namespace util {

int TypeInfoFactory::typeVal = 0;
smsc::core::synchronization::Mutex TypeInfoFactory::typeMutex;

void TypeInfoFactory::getTypeVal( int& val )
{
    smsc::core::synchronization::MutexGuard mg(typeMutex);
    if (!val) val = ++typeVal;
}

}
}
