#ifndef _INFORMER_INFOSMECOREV1_H
#define _INFORMER_INFOSMECOREV1_H

#include <memory>
#include <vector>
#include <map>
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
#include "informer/data/BindSignal.h"
#include "informer/sender/RegionSender.h"
#include "logger/Logger.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace client {
class Client;
}
}
}
}

namespace eyeline {
namespace informer {

namespace admin {
class AdminServer;
}
namespace dcp {
class DcpServer;
}

class SmscSender;
class SmscConfig;
class DeliveryMgr;
class FinalLog;

class InfosmeCoreV1 : public InfosmeCore,
public DeliveryActivator,
public smsc::core::threads::Thread
{
    class PvssRespHandler;
public:
    InfosmeCoreV1( unsigned maxsms );

    virtual ~InfosmeCoreV1();

    void init();
    void start();
    void stop();

    // --- infosmecore iface

    virtual bool isStopping() const { return stopping_; }

    virtual void addUser( const char* user );
    virtual void deleteUser( const char* login );
    virtual UserInfoPtr getUserInfo( const char* login );
    virtual void updateUserInfo( const char* login );

    virtual void addSmsc( const char* smscId );
    virtual void updateSmsc( const char* smscId );
    virtual void deleteSmsc( const char* smscId );
    virtual void updateDefaultSmsc( const char* smscId );
    virtual void getUsers( std::vector< UserInfoPtr >& users );

    virtual void addRegion( regionid_type regionId );
    virtual void updateRegion( regionid_type regionId );
    virtual void deleteRegion( regionid_type regionId );

    virtual dlvid_type addDelivery( UserInfo& userInfo,
                                    const DeliveryInfoData& info );
    virtual void deleteDelivery( const UserInfo& userInfo,
                                 dlvid_type      dlvId );
    virtual DeliveryPtr getDelivery( const UserInfo& userInfo,
                                     dlvid_type   dlvId );

    virtual int sendTestSms( const char*        sourceAddr,
                             personid_type      subscriber,
                             const char*        text,
                             bool               isFlash,
                             DlvMode            deliveryMode );

    virtual alm::IActivityLogMiner& getALM() { return *alm_; }

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

    virtual void finishStateChange( msgtime_type    currentTime,
                                    ulonglong       ymdTime,
                                    BindSignal&     bs,
                                    const Delivery& dlv );

    virtual void startPvssCheck( PvssNotifyee& notifyee, Message& msg );

    // --- end of delivery activator iface

    void selfTest();

    // smsc has just been stopped
    // virtual void notifySmscFinished( const std::string& smscId );

    void bindDeliveryRegions( const BindSignal& bs );

    /// init user stats
    void initUserStats();
    /// invoked from stats dumper
    void dumpUserStats( msgtime_type currentTime );

protected:
    /// enter main loop, exit via 'stop()'
    virtual int Execute();

    void loadUsers( const char* userId );
    void loadSmscs( const char* smscId );
    void loadRegions( regionid_type regionId );

    /// this methods has several functions:
    /// 1. create smsc: new smscId, valid cfg;
    /// 2. update smsc: old smscId, valid cfg;
    /// 3. delete smsc: old smscId, cfg=0.
    void updateSmsc( const char* smscId,
                     const SmscConfig* cfg,
                     smsc::util::config::Config* retryConfig );

private:
    smsc::logger::Logger*                      log_;
    CommonSettings                             cs_;

    smsc::core::synchronization::EventMonitor  startMon_;
    bool                                       stopping_;
    bool                                       started_;
    std::string                                     defaultSmscId_;
    smsc::core::buffers::Hash< SmscSender* >        smscs_;        // owned
    smsc::core::buffers::IntHash< RegionPtr >       regions_;      // owned
    smsc::core::buffers::IntHash< RegionSenderPtr > regSends_;   // owned
    RegionFinderV1                                  rf_;

    smsc::core::threads::ThreadPool            itp_;        // input transfer pool
    smsc::core::threads::ThreadPool            rtp_;        // resend transfer pool

    smsc::core::synchronization::Mutex            userLock_;
    smsc::core::buffers::Hash< UserInfoPtr >      users_;        // owned

    smsc::core::buffers::FastMTQueue<BindSignal>  bindQueue_;

    DeliveryMgr*                                  dlvMgr_;       // owned
    FinalLog*                                     finalLog_;     // owned
    admin::AdminServer*                           adminServer_;  // owned
    dcp::DcpServer*                               dcpServer_;    // owned
    alm::IActivityLogMiner*                       alm_;          // owned

    scag2::pvss::core::client::Client*            pvss_;         // owned
    PvssRespHandler*                              pvssHandler_;  // owned
};

} // informer
} // smsc

#endif
