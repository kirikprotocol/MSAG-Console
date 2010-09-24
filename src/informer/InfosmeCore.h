#ifndef _INFORMER_INFOSMECORE_H
#define _INFORMER_INFOSMECORE_H

#include <string>

namespace eyeline {
namespace util {
namespace config {
class ConfigView;
}
}
namespace informer {

class InfosmeCore
{
public:
    virtual ~InfosmeCore() {}

    /// notify to stop, invoked from main
    virtual void stop() = 0;

    /// configuration
    /// NOTE: do not keep a ref on cfg!
    virtual void configure( const smsc::util::config::ConfigView& cfg ) = 0;

    /// enter main loop, exit via 'stop()'
    virtual int Execute() = 0;

    // notification
    // virtual void notifySmscFinished( const std::string& smscId ) = 0;
};

} // informer
} // smsc

#endif
