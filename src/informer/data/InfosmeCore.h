#ifndef _INFORMER_INFOSMECORE_H
#define _INFORMER_INFOSMECORE_H

#include <string>
#include <vector>

#include "informer/io/Typedefs.h"

namespace smsc {
namespace util {
namespace config {
class ConfigView;
}
}
}

namespace eyeline {
namespace informer {

class RegionFinder;
class TransferTask;

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
    virtual bool isStopping() const = 0;

    /// wait a number of milliseconds, will be waked up if core is stopping
    virtual void wait( int msec ) = 0;

    /// get region finder
    virtual RegionFinder& getRegionFinder() = 0;

    virtual void selfTest() = 0;

    /// NOTE: regs will be stripped!
    virtual void deliveryRegions( dlvid_type dlvId,
                                  std::vector<regionid_type>& regs,
                                  bool bind ) = 0;

    virtual void startTransfer( TransferTask* ) = 0;
};

} // informer
} // smsc

#endif
