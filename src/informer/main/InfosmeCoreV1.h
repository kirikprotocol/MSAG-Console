#ifndef _INFORMER_INFOSMECOREV1_H
#define _INFORMER_INFOSMECOREV1_H

#include <memory>
#include <vector>
#include <map>
#include "informer/data/CommonSettings.h"
#include "informer/data/InfosmeCore.h"
#include "informer/opstore/DeliveryImpl.h"
#include "core/buffers/Hash.hpp"
#include "core/buffers/IntHash.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/Thread.hpp"
#include "core/threads/ThreadPool.hpp"
#include "logger/Logger.h"
#include "RegionFinderV1.h"

namespace smsc {
namespace util {
namespace config {
class ConfigView;
}
}
}

namespace eyeline {
namespace informer {

class SmscSender;
class RegionSender;
class SmscConfig;
class StoreJournal;
class InputJournal;
class InputMessageSource;

class InfosmeCoreV1 : public InfosmeCore, public smsc::core::threads::Thread
{
private:
    class InputJournalReader;
    class StoreJournalReader;
    class InputJournalRoller;
    class StoreJournalRoller;
public:
    static void readSmscConfig( SmscConfig& cfg,
                                const smsc::util::config::ConfigView& cv );

    InfosmeCoreV1();

    virtual ~InfosmeCoreV1();

    virtual const CommonSettings& getCS() const { return cs_; }

    virtual bool isStopping() const { return stopping_; }
    virtual void wait( int msec ) {
        if (msec<=0) return;
        smsc::core::synchronization::MutexGuard mg(startMon_);
        if (stopping_) return;
        startMon_.wait(msec);
    }

    virtual const UserInfo* getUserInfo( const char* login );

    virtual RegionFinder& getRegionFinder() { return rf_; }

    /// bind regions to delivery
    /// @param bind - true if bind, false if unbind.
    virtual void deliveryRegions( dlvid_type dlvId,
                                  std::vector<regionid_type>& regIds,
                                  bool bind );

    virtual void startTransfer( TransferTask* task );

    /// license traffic control
    virtual void incIncoming();
    virtual void incOutgoing( unsigned nchunks );

    /// final state response/receipt has been received
    virtual void receiveReceipt( const DlvRegMsgId& drmId,
                                 int smppStatus,
                                 bool retry );
    virtual bool receiveResponse( const DlvRegMsgId& drmId );

    virtual void addSmsc( const char* smscId );
    virtual void updateSmsc( const char* smscId );
    virtual void deleteSmsc( const char* smscId );
    virtual void updateDefaultSmsc();

    virtual void addRegion( regionid_type regionId );
    virtual void updateRegion( regionid_type regionId );
    virtual void deleteRegion( regionid_type regionId );

    // virtual DeliveryPtr getDelivery( dlvid_type dlvId );

    virtual void addDelivery( std::auto_ptr<DeliveryInfo> info );
    virtual void updateDelivery( std::auto_ptr<DeliveryInfo> info );
    virtual void deleteDelivery( dlvid_type dlvId );
    virtual void setDeliveryState( dlvid_type   dlvId,
                                   DlvState     newState,
                                   msgtime_type atTime = 0 );

    // --------------------

    /// configuration
    /// NOTE: do not keep a ref on cfg!
    void init( const smsc::util::config::ConfigView& cfg );

    /// notify to stop, invoked from main
    void start();
    void stop();

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

protected:
    /// enter main loop, exit via 'stop()'
    virtual int Execute();

    struct BindSignal {
        dlvid_type dlvId;
        std::vector<regionid_type> regIds;
        bool bind;
    };

    void bindDeliveryRegions( const BindSignal& bs );

private:
    smsc::logger::Logger*                      log_;
    CommonSettings                             cs_;
    smsc::core::synchronization::EventMonitor  startMon_;
    bool                                       stopping_;
    bool                                       started_;
    smsc::core::threads::ThreadPool            ttp_;        // transfer task pool
    smsc::core::buffers::Hash< SmscSender* >      smscs_;        // owned
    smsc::core::buffers::IntHash< RegionPtr >     regions_;      // owned
    smsc::core::buffers::IntHash< RegionSender* > regSends_;     // owned

    typedef std::list<DeliveryImplPtr> DeliveryList;
    typedef smsc::core::buffers::IntHash< DeliveryList::iterator > DeliveryHash;
    typedef std::multimap< msgtime_type, dlvid_type >       DeliveryWakeQueue;

    DeliveryHash                                  deliveryHash_;
    DeliveryList                                  deliveryList_;
    DeliveryWakeQueue                             deliveryWakeQueue_;
    DeliveryList::iterator                        inputRollingIter_;
    DeliveryList::iterator                        storeRollingIter_;
    
    StoreJournal*                                 storeJournal_; // owned
    InputJournal*                                 inputJournal_; // owned
    smsc::core::synchronization::Mutex            bindQueueLock_;
    smsc::core::buffers::CyclicQueue<BindSignal>  bindQueue_;
    RegionFinderV1                                rf_;
    InputJournalRoller*                           inputRoller_;  // owned
    StoreJournalRoller*                           storeRoller_;  // owned
};

} // informer
} // smsc

#endif
