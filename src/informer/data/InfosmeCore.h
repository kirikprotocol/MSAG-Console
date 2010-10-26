#ifndef _INFORMER_INFOSMECORE_H
#define _INFORMER_INFOSMECORE_H

#include <string>
#include <vector>

#include "informer/io/Typedefs.h"
#include "Delivery.h"

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

    virtual const CommonSettings& getCS() const = 0;

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
    virtual void receiveReceipt( const DlvRegMsgId& drmId, int  smppStatus, bool retry ) = 0;
    /// response (non-final) has been received
    virtual bool receiveResponse( const DlvRegMsgId& drmId ) = 0;

    /// create/update smsc
    virtual void addSmsc( const char* smscId ) = 0;
    virtual void updateSmsc( const char* smscId ) = 0;
    virtual void deleteSmsc( const char* smscId ) = 0;
    virtual void updateDefaultSmsc() = 0;

    /// reload regions
    virtual void addRegion( regionid_type regionId ) = 0;
    virtual void updateRegion( regionid_type regionId ) = 0;
    virtual void deleteRegion( regionid_type regionId ) = 0;

    // virtual DeliveryPtr getDelivery( dlvid_type dlvId ) = 0;
    virtual void addDelivery( std::auto_ptr<DeliveryInfo> info ) = 0;
    virtual void updateDelivery( std::auto_ptr<DeliveryInfo> info ) = 0;
    virtual void deleteDelivery( dlvid_type dlvId ) = 0;
    virtual void setDeliveryState( dlvid_type   dlvId,
                                   DlvState     newState,
                                   msgtime_type atTime = 0 ) = 0;
};

} // informer
} // smsc

#endif
