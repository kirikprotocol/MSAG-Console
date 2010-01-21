#include "NotificationManager.h"

namespace scag2 {
namespace counter {
namespace impl {

NotificationManager::NotificationManager() :
log_(smsc::logger::Logger::getInstance("cnt.nmgr"))
{
    smsc_log_debug(log_,"ctor");
}


NotificationManager::~NotificationManager()
{
    smsc_log_debug(log_,"dtor");
}


void NotificationManager::advanceTime( usec_type curTime )
{
    smsc_log_debug(log_,"advanceTime %lld", curTime);
}

}
}
}
