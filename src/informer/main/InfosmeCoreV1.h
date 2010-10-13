#ifndef _INFORMER_INFOSMECOREV1_H
#define _INFORMER_INFOSMECOREV1_H

#include <memory>
#include <vector>
#include "informer/data/CommonSettings.h"
#include "informer/data/Region.h"
#include "informer/data/InfosmeCore.h"
#include "informer/opstore/Delivery.h"
#include "core/buffers/Hash.hpp"
#include "core/buffers/IntHash.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/Thread.hpp"
#include "core/threads/ThreadPool.hpp"
#include "logger/Logger.h"

namespace eyeline {
namespace informer {

class SmscSender;
class RegionSender;
class SmscConfig;
class StoreJournal;
class InputJournal;
class InputMessageSource;

class InfosmeCoreV1 : public InfosmeCore, public RegionFinder, public smsc::core::threads::Thread
{
private:
    class InputJournalReader;
    class StoreJournalReader;
public:
    static void readSmscConfig( SmscConfig& cfg,
                                const smsc::util::config::ConfigView& cv );

    InfosmeCoreV1();

    virtual ~InfosmeCoreV1();

    /// configuration
    /// NOTE: do not keep a ref on cfg!
    virtual void init( const smsc::util::config::ConfigView& cfg );

    /// notify to stop, invoked from main
    virtual void start();
    virtual void stop();
    virtual bool isStopping() const { return stopping_; }

    virtual RegionFinder& getRegionFinder() { return *this; }

    virtual void selfTest();

    /// bind regions to delivery
    /// @param bind - true if bind, false if unbind.
    virtual void deliveryRegions( dlvid_type dlvId,
                                  std::vector<regionid_type>& regIds,
                                  bool bind );

    virtual void startTransfer( TransferTask* task );

    // --------------------

    // smsc has just been stopped
    // virtual void notifySmscFinished( const std::string& smscId );

    /// this methods has several functions:
    /// 1. create smsc: new smscId, valid cfg;
    /// 2. update smsc: old smscId, valid cfg;
    /// 3. delete smsc: old smscId, cfg=0.
    void updateSmsc( const std::string& smscId, const SmscConfig* cfg );

    /// reload all regions
    void reloadRegions( const std::string& defaultSmscId );

    virtual regionid_type findRegion( personid_type subscriber );

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
    smsc::core::buffers::IntHash< DeliveryPtr >   deliveries_;   // owned
    StoreJournal*                                 storeJournal_; // owned
    InputJournal*                                 inputJournal_; // owned
    smsc::core::synchronization::Mutex            bindQueueLock_;
    smsc::core::buffers::CyclicQueue<BindSignal>  bindQueue_;
};

} // informer
} // smsc

#endif
