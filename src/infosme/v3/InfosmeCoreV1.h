#ifndef _INFOSME_V3_INFOSMECOREV1_H
#define _INFOSME_V3_INFOSMECOREV1_H

#include "InfosmeCore.h"

namespace smsc {
namespace infosme {

class InfosmeCoreV1 : public InfosmeCore
{
public:
    /// notify to stop, invoked from main
    virtual void stop();

    /// configuration
    /// NOTE: do not keep a ref on cfg!
    virtual void configure( const smsc::util::config::Config& cfg );

    /// enter main loop, exit via 'stop()'
    virtual int Execute();

};

} // infosme
} // smsc

#endif
