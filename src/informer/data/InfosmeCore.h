#ifndef _INFORMER_INFOSMECORE_H
#define _INFORMER_INFOSMECORE_H

#include <string>
#include <vector>

#include "informer/io/Typedefs.h"
#include "Delivery.h"
#include "UserInfo.h"

namespace eyeline {
namespace informer {

class CommonSettings;
class RegionFinder;
class InputTransferTask;
class ResendTransferTask;

class InfosmeCore
{
public:
    virtual ~InfosmeCore() {}

    virtual const CommonSettings& getCS() const = 0;

    /// check if stopping
    virtual bool isStopping() const = 0;

    /// get user info (a stub for now)
    virtual void addUser( const char* user ) = 0;
    virtual void deleteUser( const char* login ) = 0;
    virtual UserInfoPtr getUserInfo( const char* login ) = 0;
    virtual void updateUserInfo( const char* login ) = 0;

    /// create/update smsc
    virtual void addSmsc( const char* smscId ) = 0;
    virtual void updateSmsc( const char* smscId ) = 0;
    virtual void deleteSmsc( const char* smscId ) = 0;
    virtual void updateDefaultSmsc( const char* smscId ) = 0;

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
