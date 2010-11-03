#ifndef _INFORMER_INFOSMECOREV1_H
#define _INFORMER_INFOSMECOREV1_H

#include <memory>
#include <vector>
#include <map>
#include "BindSignal.h"
#include "RegionFinderV1.h"
#include "core/buffers/Hash.hpp"
#include "core/buffers/IntHash.hpp"
#include "core/buffers/FastMTQueue.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/Thread.hpp"
#include "core/threads/ThreadPool.hpp"
#include "informer/data/CommonSettings.h"
#include "informer/data/DeliveryActivator.h"
#include "informer/data/InfosmeCore.h"
#include "logger/Logger.h"

namespace smsc {
namespace util {
namespace config {
class ConfigView;
}
}
}

namespace eyeline {
namespace informer {

namespace admin {
class AdminServer;
}

class SmscSender;
class RegionSender;
class SmscConfig;
class DeliveryMgr;

class InfosmeCoreV1 : public InfosmeCore,
public DeliveryActivator,
public smsc::core::threads::Thread
{
public:
    static void readSmscConfig( SmscConfig& cfg,
                                const smsc::util::config::ConfigView& cv );

    InfosmeCoreV1();

    virtual ~InfosmeCoreV1();

    /// configuration
    /// NOTE: do not keep a ref on cfg!
    void init( const smsc::util::config::ConfigView& cfg );
    void start();
    void stop();

    // --- infosmecore iface

    virtual const CommonSettings& getCS() const { return cs_; }
    virtual bool isStopping() const { return stopping_; }

    virtual void addUser( const char* user );
    virtual void deleteUser( const char* login );
    virtual UserInfoPtr getUserInfo( const char* login );
    virtual void updateUserInfo( const char* login );

    virtual void addSmsc( const char* smscId );
    virtual void updateSmsc( const char* smscId );
    virtual void deleteSmsc( const char* smscId );
    virtual void updateDefaultSmsc( const char* smscId );

    virtual void addRegion( regionid_type regionId );
    virtual void updateRegion( regionid_type regionId );
    virtual void deleteRegion( regionid_type regionId );

    virtual dlvid_type addDelivery( UserInfo& userInfo,
                                    const DeliveryInfoData& info );
    virtual void deleteDelivery( const UserInfo& userInfo,
                                 dlvid_type      dlvId );
    virtual DeliveryPtr getDelivery( const UserInfo& userInfo,
                                     dlvid_type   dlvId );

    // --- end of infosme core iface

    // --- delivery activator iface

    // bool isStopping() above

    virtual RegionFinder& getRegionFinder() { return rf_; }

    /// bind regions to delivery
    /// @param bind - true if bind, false if unbind.
    virtual void deliveryRegions( dlvid_type dlvId,
                                  std::vector<regionid_type>& regIds,
                                  bool bind );

    virtual void startInputTransfer( InputTransferTask* task ) {
        itp_.startTask(task);
    }
    virtual void startResendTransfer( ResendTransferTask* task ) {
        rtp_.startTask(task);
    }

    virtual void finishStateChange( ulonglong    ymdTime,
                                    Delivery&    dlv,
                                    DlvState     oldState );

    /*
     // virtual void logStateChange( ulonglong   ymd,
                                 dlvid_type  dlvId,
                                 const char* userId,
                                 DlvState    newState,
                                 unsigned    planTime );
     */

    // --- end of delivery activator iface

    void selfTest();

    // smsc has just been stopped
    // virtual void notifySmscFinished( const std::string& smscId );

    /// this methods has several functions:
    /// 1. create smsc: new smscId, valid cfg;
    /// 2. update smsc: old smscId, valid cfg;
    /// 3. delete smsc: old smscId, cfg=0.
    void updateSmsc( const std::string& smscId, const SmscConfig* cfg );

    /// reload all regions
    void reloadRegions( const std::string& defaultSmscId );

    void bindDeliveryRegions( const BindSignal& bs );

    /// invoked from stats dumper
    void dumpUserStats( msgtime_type currentTime );

protected:
    /// enter main loop, exit via 'stop()'
    virtual int Execute();

private:
    smsc::logger::Logger*                      log_;
    CommonSettings                             cs_;
    smsc::core::synchronization::EventMonitor  startMon_;
    bool                                       stopping_;
    bool                                       started_;
    smsc::core::threads::ThreadPool            itp_;        // input transfer pool
    smsc::core::threads::ThreadPool            rtp_;        // resend transfer pool
    smsc::core::buffers::Hash< UserInfoPtr >      users_;        // owned
    smsc::core::buffers::Hash< SmscSender* >      smscs_;        // owned
    smsc::core::buffers::IntHash< RegionPtr >     regions_;      // owned
    smsc::core::buffers::IntHash< RegionSender* > regSends_;     // owned

    smsc::core::buffers::FastMTQueue<BindSignal>  bindQueue_;

    RegionFinderV1                                rf_;

    DeliveryMgr*                                  dlvMgr_;       // owned
    admin::AdminServer*                           adminServer_;

    smsc::core::synchronization::Mutex            logStateLock_;
    ulonglong                                     logStateTime_;
    FileGuard                                     logStateFile_;
};

} // informer
} // smsc

#endif
