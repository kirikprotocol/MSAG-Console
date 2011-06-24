#ifndef _INFORMER_REGIONALSTORAGE_H
#define _INFORMER_REGIONALSTORAGE_H

#include <map>
#include "core/buffers/CyclicQueue.hpp"
#include "core/buffers/IntHash64.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/synchronization/Condition.hpp"
#include "informer/io/EmbedRefPtr.h"
#include "informer/io/RelockMutexGuard.h"
#include "informer/io/SpeedControl.h"
#include "informer/data/InputMessageSource.h"
#include "informer/data/Region.h"
#include "logger/Logger.h"
#include "RolledList.h"

namespace smsc {
namespace sms {
struct SMS;
}
}

namespace eyeline {
namespace informer {

class StoreJournal;
class DeliveryImpl;
class RetryPolicy;
class DeliveryInfo;

/// Working storage for messages for one Delivery/Region
class RegionalStorage : protected TransferRequester
{
private:
    friend class EmbedRefPtr<RegionalStorage>;

    /// this one is used to increase granularity, i.e. to lower the lock contention
    static const unsigned CONDITION_COUNT = 13;

    /// ALL messages are kept in this list
    /// The message may be locked from modification for the time of serialization.
    typedef RolledList< MessageLocker, MessageList >    MsgList;
    // typedef MsgList::RollIter                           MsgIter;
#ifdef MAGICTYPECHECK
    struct                                              MsgLock;
#else
    typedef MsgList::ItemLock                           MsgLock;
#endif
    typedef std::multimap< msgtime_type, MsgIter >      ResendQueue;
    typedef smsc::core::buffers::IntHash64< MsgIter >   MessageHash;
    typedef smsc::core::buffers::CyclicQueue< MsgIter > NewQueue;

    // message locking guard fwd decl
    // class StopRollingGuard;

public:

    RegionalStorage( DeliveryImpl&       dlv,
                     const RegionPtr&    region,
                     msgtime_type        nextResendFile = 0 );

    virtual ~RegionalStorage();
    dlvid_type getDlvId() const;
    regionid_type getRegionId() const { return region_->getRegionId(); }
    const DeliveryInfo& getDlvInfo() const;
    DeliveryInfo& getDlvInfo();
    const Region& getRegion() const { return *region_; }

    DlvState getState() const;

    /// return true if there is no messages in new and resend
    bool isFinished();

    bool isEndDateReached( msgtime_type currentTime );

    // get the message with given messageid.
    // this method accesses active messages only.
    // bool getMessage( msgid_type msgId, Message& msg );

    /// get the next message.
    /// this method accesses new/resent messages only.
    /// @param currentTime - current GMT;
    /// @param weekTime - the local time (seconds) since monday midnight.
    /// SIDEEFFECT: request more messages into the new queue if needed.
    /// @return how many microseconds to wait until message will be ready:
    ///   <0 message is ready (return number of seconds to wait until activeEnd).
    ///   =0 message is ready (not limited by active period).
    ///   >0 message is not ready (number of MICROseconds to wait).
    int getNextMessage( usectime_type currentTime,
                        int weekTime, Message& msg );

    /// change message state when non-transaction response has come.
    /// message is left in the cache.
    void messageSent( msgid_type msgId,
                      msgtime_type currentTime );

    /// change message state when temporal failure received.
    /// message is removed from the cache.
    void retryMessage( msgid_type         msgId,
                       const RetryPolicy& policy,
                       msgtime_type       currentTime,
                       int                smppState,
                       unsigned           nchunks );

    /// finalize message. message is removed from cache.
    void finalizeMessage( msgid_type   msgId,
                          msgtime_type currentTime,
                          uint8_t      state,
                          int          smppState,
                          unsigned     nchunks );


    /// notify transfer threads that they should be stopped.
    void stopTransfer();

    /// rolling over the storage
    /// @return number of bytes written
    size_t rollOver( SpeedControl< usectime_type, tuPerSec >& speedControl );

    /// invoked at init stage ONLY!
    void setRecordAtInit( Message& msg, regionid_type serial );

    /// invoked after opstore is loaded.
    /// messages accumulated in messageHash_ will be partly moved according to their state
    /// into newQueue and resendQueue.
    /// @return true if there are messages to send.
    bool postInit();

    /// cancel operative storage messages.
    void cancelOperativeStorage();

protected:
    /// invoked when upload task has finished.
    virtual void transferFinished( InputTransferTask* );

    /// invoked when upload task has finished.
    virtual void transferFinished( ResendTransferTask* );

    /// add new messages to processing.
    virtual void addNewMessages( msgtime_type currentTime,
                                 MessageList& listFrom );
    // MsgIter      iter1,
    // MsgIter      iter2 );

    /// NOTE: mg must be locked!
    void doFinalize( MsgLock&          ml,
                     msgtime_type      currentTime,
                     uint8_t           state,
                     int               smppState,
                     unsigned          nchunks );

    virtual void resendIO( bool isInputDirection, volatile bool& stopFlag );

    // void makeResendFilePath( char*     fpath,
    // ulonglong nextTime );

    // return the next resend file or 0
    // msgtime_type findNextResendFile();

private:
    // message cleanup
    // void destroy( Message& msg );

private:
    smsc::logger::Logger*                     log_;

    smsc::core::synchronization::EventMonitor cacheMon_;

    // the list of all messages
    MsgList                                   messageList_;

    // iterator that points to item that will be stored automatically from rollOver()
    // MsgIter                                   storingIter_;

    /// The message which is stored in the message list
    /// is pointed to from ONLY ONE of messageHash, resendQueue or newQueue!
    /// e.g. they are exclusive.
    bool                              cancelling_;
    MessageHash                       messageHash_;
    ResendQueue                       resendQueue_;
    NewQueue                          newQueue_;

    EmbedRefPtr<DeliveryImpl>         dlv_;
    InputTransferTask*                inputTransferTask_;  // owned
    ResendTransferTask*               resendTransferTask_; // owned
    RegionPtr                         region_;
    // unsigned                          stopRolling_;        // wait until 0 to roll
    
    unsigned                           newOrResend_; // if <3 then new, otherwise resend

    // they are used to average input speed
    usectime_type                      inputRequestGrantTime_;
    unsigned                           numberOfInputReqGrant_;

    /// the next resend file starting time or 0 (if there is no files).
    msgtime_type                       nextResendFile_;

    // smsc::core::synchronization::Condition conds_[CONDITION_COUNT];
};

typedef EmbedRefPtr< RegionalStorage > RegionalStoragePtr;

}
}

#endif /* !_SCAG_PROTOTYPES_INFOSME_MESSAGEFILE_H */
