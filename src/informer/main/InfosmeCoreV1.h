#ifndef _INFORMER_INFOSMECOREV1_H
#define _INFORMER_INFOSMECOREV1_H

#include <memory>
#include "informer/data/Region.h"
#include "informer/opstore/Delivery.h"
#include "informer/opstore/InfosmeCore.h"
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
class InputMessageSource;

class InfosmeCoreV1 : public InfosmeCore, public smsc::core::threads::Thread
{
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

    // smsc has just been stopped
    // virtual void notifySmscFinished( const std::string& smscId );

    /// this methods has several functions:
    /// 1. create smsc: new smscId, valid cfg;
    /// 2. update smsc: old smscId, valid cfg;
    /// 3. delete smsc: old smscId, cfg=0.
    void updateSmsc( const std::string& smscId, const SmscConfig* cfg );

    /// reload all regions
    void reloadRegions();

    /// update delivery
    /// 1. create delivery: new dlvId, valid dlvInfo;
    /// 2. update delivery: old dlvId, valid dlvInfo;
    /// 3. delete delivery: old dlvId, dlvInfo=0.
    void updateDelivery( dlvid_type dlvId, std::auto_ptr<DeliveryInfo>& dlvInfo );

protected:
    /// enter main loop, exit via 'stop()'
    virtual int Execute();

private:
    smsc::logger::Logger*                      log_;
    smsc::core::synchronization::EventMonitor  startMon_;
    bool                                       stopping_;
    bool                                       started_;
    smsc::core::threads::ThreadPool            tp_;
    smsc::core::buffers::Hash< SmscSender* >      smscs_;   // owned
    smsc::core::buffers::IntHash< RegionPtr >     regions_; // owned
    smsc::core::buffers::IntHash< RegionSender* > regSends_; // owned
    smsc::core::buffers::IntHash< DeliveryPtr >   deliveries_; // owned
    StoreJournal*                                 storeLog_;   // owned
    InputMessageSource*                           messageSource_;
};

} // informer
} // smsc

#endif
