#include "SCAGCommand2.h"

namespace {
uint32_t                            serial;
smsc::core::synchronization::Mutex  mtx;
}

namespace scag2 {
namespace transport {

uint32_t SCAGCommand::makeSerial()
{
    smsc::core::synchronization::MutexGuard mg(mtx);
    if ( ++serial < 10 ) serial = 10;
    return serial;
}

}
}
