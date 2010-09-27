#ifndef _INFORMER_INFOSMECORE_H
#define _INFORMER_INFOSMECORE_H

#include <string>

namespace smsc {
namespace util {
namespace config {
class ConfigView;
}
}
}

namespace eyeline {
namespace informer {

class InfosmeCore
{
public:
    virtual ~InfosmeCore() {}

    /// init the core, should be invoked before start()
    /// NOTE: do not keep a ref on cfg!
    virtual void init( const smsc::util::config::ConfigView& cfg ) = 0;

    /// notify to stop, invoked from main
    virtual void start() = 0;
    virtual void stop() = 0;
};

} // informer
} // smsc

#endif
