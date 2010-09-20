#include "InfosmeCoreV1.h"

namespace smsc {
namespace infosme {

InfosmeCoreV1::InfosmeCoreV1() :
log_(0),
stopping_(true)
{
    log_ = smsc::logger::Logger::getInstance("is.core");
}


void InfosmeCoreV1::stop()
{
    if (stopping_) return;
    MutexGuard mg(stopMon_);
    if (stopping_) return;
    smsc_log_info(log_,"stop() received");
    stopping_ = true;
    stopMon_.notifyAll();
}


void InfosmeCoreV1::configure( const smsc::util::config::ConfigView& cfg )
{
    smsc_log_info(log_,"configuring InfosmeCore");
    // FIXME
}


int InfosmeCoreV1::Execute()
{
    {
        if (!stopping_) return 0;
        MutexGuard mg(stopMon_);
        if (!stopping_) return 0;
        stopping_ = false;
    }
    smsc_log_info(log_,"starting main loop");
    while (!stopping_) {
        MutexGuard mg(stopMon_);
        if (stopping_) break;
        // FIXME
    }
    smsc_log_info(log_,"finishing main loop");
    return 0;
}

}
}
