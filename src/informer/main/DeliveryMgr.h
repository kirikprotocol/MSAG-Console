#ifndef _INFORMER_DELIVERYMGR_H
#define _INFORMER_DELIVERYMGR_H

#include <memory>
#include "core/threads/Thread.hpp"
#include "core/threads/ThreadPool.hpp"
#include "informer/data/DeliveryInfo.h"
#include "informer/data/SpeedControl.h"
#include "informer/io/Typedefs.h"
#include "informer/newstore/InputJournal.h"
#include "informer/opstore/DeliveryImpl.h"
#include "informer/opstore/StoreJournal.h"

namespace eyeline {
namespace informer {

class InfosmeCoreV1;
class CommonSettings;

class DeliveryMgr : public smsc::core::threads::Thread
{
    class InputJournalReader;
    class StoreJournalReader;
    class InputJournalRoller;
    class StoreJournalRoller;
    class StatsDumper;
    class CancelTask;
    class DeliveryChunkList;

public:
    DeliveryMgr( InfosmeCoreV1& core, CommonSettings& cs );

    virtual ~DeliveryMgr();

    void init();
    void start();
    void stop();

    dlvid_type createDelivery( UserInfo& userInfo,
                               const DeliveryInfoData& info );
    void deleteDelivery( dlvid_type dlvId,
                         bool moveToArchive );

    /// enter main loop, exit via 'stop()'
    virtual int Execute();

    bool getDelivery( dlvid_type dlvId, DeliveryImplPtr& ptr );

    dlvid_type getDeliveries( unsigned        count,
                              DeliveryFilter& filter,
                              DeliveryList*   result,
                              dlvid_type      startId );

    /// invoked to from infosmecorev1 only!
    bool finishStateChange( msgtime_type    currentTime,
                            ulonglong       ymdTime,
                            const Delivery& dlv );

    /// this methods adds a threaded task which is going to cancel
    /// all messages in the delivery.
    void startCancelThread( dlvid_type dlvId );

protected:
    /// get delivery w/o automatic reading from disk
    bool innerGetDelivery( dlvid_type dlvId, DeliveryImplPtr& ptr );

    /// used internally
    void addDelivery( DeliveryInfo*    info,
                      DlvState         state,
                      msgtime_type     planTime,
                      bool             checkDlvLimit,
                      DeliveryImplPtr* ptr = 0 );

    dlvid_type getNextDlvId();
    
    /// signal archive, optionally writing delivery id
    void signalArchive( dlvid_type dlvId = 0 );

    /// read signals from archive
    void readFromArchive();

    void readDelivery( dlvid_type dlvId, DeliveryImplPtr* ptr = 0 );

    typedef std::list<DeliveryImplPtr>                              DeliveryIList;
    typedef smsc::core::buffers::IntHash< DeliveryIList::iterator > DeliveryHash;
    typedef std::multimap< msgtime_type, dlvid_type >         DeliveryWakeQueue;

    DeliveryIList::iterator popDelivery( dlvid_type dlvId,
                                         DeliveryIList& tokill );


    inline void freeDlvIterator( DeliveryIList::iterator iter ) {
        if (inputRollingIter_ == iter) ++inputRollingIter_;
        if (storeRollingIter_ == iter) ++storeRollingIter_;
        if (statsDumpingIter_ == iter) ++statsDumpingIter_;
    }

private:
    smsc::logger::Logger*                      log_;
    InfosmeCoreV1&                             core_;
    CommonSettings&                            cs_;

    DeliveryChunkList*                            deliveryChunkList_;

    smsc::core::synchronization::EventMonitor  mon_;
    DeliveryHash                                  deliveryHash_;
    DeliveryIList                                 deliveryList_;
    DeliveryWakeQueue                             deliveryWakeQueue_; // on plantime
    DeliveryIList::iterator                       inputRollingIter_;
    DeliveryIList::iterator                       storeRollingIter_;
    DeliveryIList::iterator                       statsDumpingIter_;
    
    StoreJournal*                                 storeJournal_; // owned
    InputJournal*                                 inputJournal_; // owned

    InputJournalRoller*                           inputRoller_;  // owned
    StoreJournalRoller*                           storeRoller_;  // owned
    StatsDumper*                                  statsDumper_;  // owned

    smsc::core::synchronization::Mutex            logStateLock_;
    ulonglong                                     logStateTime_;
    FileGuard                                     logStateFile_;

    dlvid_type                                    lastDlvId_;

    smsc::core::threads::ThreadPool               ctp_;

    smsc::core::synchronization::Mutex            archiveLock_;
};

} // informer
} // smsc

#endif
