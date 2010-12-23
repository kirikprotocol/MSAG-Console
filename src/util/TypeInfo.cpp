#include "TypeInfo.h"
#include "crc32.h"

namespace smsc {
namespace util {

int TypeInfoFactory::typeVal = 0;
smsc::core::synchronization::Mutex TypeInfoFactory::typeMutex;

bool TypeInfoFactory::setTypeVal( int& val )
{
    smsc::core::synchronization::MutexGuard mg(typeMutex);
    if (!val) {
        val = ++typeVal;
        return true;
    }
    return false;
}


bool TypeInfoFactory::setMagicVal( int val, int& magic )
{
    smsc::core::synchronization::MutexGuard mg(typeMutex);
    if (magic) { return false; }
    uint32_t res = 0xcecececeU;
    do {
        res = crc32(res,&val,sizeof(val));
    } while ( res == 0 );
    magic = int(res);
    return true;
}

}
}
