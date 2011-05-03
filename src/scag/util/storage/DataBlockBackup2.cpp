#include "DataBlockBackup2.h"
#include "core/synchronization/Mutex.hpp"

namespace {
bool logInited = false;
smsc::core::synchronization::Mutex logMutex;
}

namespace scag2 {
namespace util {
namespace storage {

smsc::logger::Logger* DataBlockBackup2Base::log_ = 0;

void DataBlockBackup2Base::initLog()
{
    if ( logInited ) return;
    smsc::core::synchronization::MutexGuard mg(logMutex);
    if ( logInited ) return;
    log_ = smsc::logger::Logger::getInstance("dbb2.alloc");
}

}
}
}
