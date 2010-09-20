#ifndef _INFOSME_V3_INFOSMECOREV1_H
#define _INFOSME_V3_INFOSMECOREV1_H

#include "InfosmeCore.h"
#include "logger/Logger.h"
#include "core/synchronization/EventMonitor.hpp"

namespace smsc {
namespace infosme {

class InfosmeCoreV1 : public InfosmeCore
{
public:
    InfosmeCoreV1();

    /// notify to stop, invoked from main
    virtual void stop();

    /// configuration
    /// NOTE: do not keep a ref on cfg!
    virtual void configure( const smsc::util::config::ConfigView& cfg );

    /// enter main loop, exit via 'stop()'
    virtual int Execute();

private:
    smsc::core::synchronization::EventMonitor stopMon_;
    smsc::logger::Logger* log_;
    bool stopping_;
};

} // infosme
} // smsc

#endif
