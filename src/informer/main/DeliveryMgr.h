#ifndef _INFORMER_DELIVERYMGR_H
#define _INFORMER_DELIVERYMGR_H

#include <memory>
#include "core/threads/Thread.hpp"
#include "informer/data/DeliveryInfo.h"
#include "informer/io/Typedefs.h"
#include "informer/newstore/InputJournal.h"
#include "informer/opstore/DeliveryImpl.h"
#include "informer/opstore/StoreJournal.h"
#include "informer/sender/ReceiptProcessor.h"

namespace eyeline {
namespace informer {

class InfosmeCoreV1;
class CommonSettings;

class DeliveryMgr : public ReceiptProcessor, public smsc::core::threads::Thread
{
    class InputJournalReader;
    class StoreJournalReader;
    class InputJournalRoller;
    class StoreJournalRoller;
    class StatsDumper;

public:
    DeliveryMgr( InfosmeCoreV1& core, CommonSettings& cs );

    virtual ~DeliveryMgr();

    void init();
    void start();
    void stop();
    inline bool isCoreStopping() const;

    /// final state response/receipt has been received
    virtual void receiveReceipt( const DlvRegMsgId& drmId,
                                 const RetryPolicy& retryPolicy,
                                 int smppStatus, bool retry,
                                 unsigned nchunks );
    virtual bool receiveResponse( const DlvRegMsgId& drmId );

    /// license traffic control
    virtual void incIncoming();
    virtual void incOutgoing( unsigned nchunks );

    // --- end of receipt processor iface

    dlvid_type createDelivery( UserInfo& userInfo,
                               const DeliveryInfoData& info );
    void deleteDelivery( dlvid_type dlvId,
                         std::vector<regionid_type>& regIds );

    /// enter main loop, exit via 'stop()'
    virtual int Execute();

    inline bool getDelivery( dlvid_type dlvId, DeliveryImplPtr& ptr )
    {
        MutexGuard mg(mon_);
        DeliveryList::iterator* iter = deliveryHash_.GetPtr(dlvId);
        if (!iter) return false;
        ptr = **iter;
        return true;
    }

    /// invoked to from infosmecorev1 only!
    bool finishStateChange( msgtime_type    currentTime,
                            ulonglong       ymdTime,
                            const Delivery& dlv );

protected:
    /// used internally
    void addDelivery( UserInfo&     userInfo,
                      DeliveryInfo* info,
                      DlvState      state,
                      msgtime_type  planTime );

    dlvid_type getNextDlvId();
    
private:
    smsc::logger::Logger*                      log_;
    InfosmeCoreV1&                             core_;
    CommonSettings&                            cs_;
    smsc::core::synchronization::EventMonitor  mon_;

    bool                                       stopping_;

    typedef std::list<DeliveryImplPtr>                             DeliveryList;
    typedef smsc::core::buffers::IntHash< DeliveryList::iterator > DeliveryHash;
    typedef std::multimap< msgtime_type, dlvid_type >         DeliveryWakeQueue;

    DeliveryHash                                  deliveryHash_;
    DeliveryList                                  deliveryList_;
    DeliveryWakeQueue                             deliveryWakeQueue_; // on plantime
    DeliveryWakeQueue                             deliveryStopQueue_; // on activeend
    DeliveryList::iterator                        inputRollingIter_;
    DeliveryList::iterator                        storeRollingIter_;
    DeliveryList::iterator                        statsDumpingIter_;
    
    StoreJournal*                                 storeJournal_; // owned
    InputJournal*                                 inputJournal_; // owned

    InputJournalRoller*                           inputRoller_;  // owned
    StoreJournalRoller*                           storeRoller_;  // owned
    StatsDumper*                                  statsDumper_;  // owned

    smsc::core::synchronization::Mutex            logStateLock_;
    ulonglong                                     logStateTime_;
    FileGuard                                     logStateFile_;
    dlvid_type                                    lastDlvId_;
};

} // informer
} // smsc

#endif
