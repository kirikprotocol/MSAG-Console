#ifndef _INFOSME_V3_INFOSMECORE_H
#define _INFOSME_V3_INFOSMECORE_H

namespace smsc {
namespace util {
namespace config {
class Config;
}
}
namespace infosme {

class InfosmeCore
{
public:
    virtual ~InfosmeCore() {}

    /// notify to stop, invoked from main
    virtual void stop() = 0;

    /// configuration
    /// NOTE: do not keep a ref on cfg!
    virtual void configure( const smsc::util::config::Config& cfg ) = 0;

    /// enter main loop, exit via 'stop()'
    virtual int Execute() = 0;

};

} // infosme
} // smsc

#endif
