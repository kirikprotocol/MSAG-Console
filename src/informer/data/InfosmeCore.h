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

class CommonSettings;
class RegionFinder;
class TransferTask;
class UserInfo;

class InfosmeCore
{
public:
    virtual ~InfosmeCore() {}

    virtual const CommonSettings& getCommonSettings() const = 0;

    /// check if stopping
    virtual bool isStopping() const = 0;

    /// wait a number of milliseconds, will be waked up if core is stopping
    virtual void wait( int msec ) = 0;

    /// get user info (a stub for now)
    virtual const UserInfo* getUserInfo( const char* login ) = 0;

    /// get region finder
    virtual RegionFinder& getRegionFinder() = 0;

    /// NOTE: vector regs will be emptied!
    /// @param bind: bind dlv and regs if true, unbind otherwise.
    virtual void deliveryRegions( dlvid_type dlvId,
                                  std::vector<regionid_type>& regs,
                                  bool bind ) = 0;

    /// start a task transferring messages from inputstore into opstore.
    virtual void startTransfer( TransferTask* ) = 0;

    /// traffic limitation by license
    virtual void incIncoming() = 0;
    virtual void incOutgoing( unsigned nchunks ) = 0;

    /// final receipt/response has been received
    virtual void receiveResponse( const DlvRegMsgId& drmId, int smppStatus, bool retry ) = 0;
};

} // informer
} // smsc

#endif
