#ifndef _INFORMER_INFOSMECOREV1_H
#define _INFORMER_INFOSMECOREV1_H

#include <memory>
#include <vector>
#include "informer/data/CommonSettings.h"
#include "informer/data/InfosmeCore.h"
#include "informer/opstore/Delivery.h"
#include "core/buffers/Hash.hpp"
#include "core/buffers/IntHash.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/Thread.hpp"
#include "core/threads/ThreadPool.hpp"
#include "logger/Logger.h"
#include "RegionFinderV1.h"

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

    virtual const CommonSettings& getCommonSettings() const { return cs_; }

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
    virtual void receiveResponse( const DlvRegMsgId& drmId, int smppStatus, bool retry );

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

    /// update delivery
    /// 1. create delivery: new dlvId, valid dlvInfo;
    /// 2. update delivery: old dlvId, valid dlvInfo;
    /// 3. delete delivery: old dlvId, dlvInfo=0.
    void updateDelivery( dlvid_type dlvId, std::auto_ptr<DeliveryInfo>& dlvInfo );

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

    typedef std::list<DeliveryPtr> DeliveryList;
    typedef smsc::core::buffers::IntHash< DeliveryList::iterator > DeliveryHash;
    DeliveryHash                                  deliveryHash_;
    DeliveryList                                  deliveryList_;
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
