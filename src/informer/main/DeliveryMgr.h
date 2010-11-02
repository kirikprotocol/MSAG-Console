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

    // --- receipt processor iface
    virtual const CommonSettings& getCS() const { return cs_; }

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

    void addDelivery( DeliveryInfo* info );
    void updateDelivery( DeliveryInfo* info );
    void deleteDelivery( dlvid_type dlvId, std::vector<regionid_type>& regIds );
    void setDeliveryState( dlvid_type   dlvId,
                           DlvState     newState,
                           msgtime_type atTime,
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
    DeliveryWakeQueue                             deliveryWakeQueue_;
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
    FileGuard                                     logStateCur_;
    FileGuard                                     logStateOld_;
};

} // informer
} // smsc

#endif
