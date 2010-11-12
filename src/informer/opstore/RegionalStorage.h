#ifndef _INFORMER_REGIONALSTORAGE_H
#define _INFORMER_REGIONALSTORAGE_H

#include <map>
#include "core/buffers/CyclicQueue.hpp"
#include "core/buffers/IntHash64.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/synchronization/Condition.hpp"
#include "informer/io/EmbedRefPtr.h"
#include "informer/io/RelockMutexGuard.h"
#include "informer/data/InputMessageSource.h"
#include "logger/Logger.h"

namespace smsc {
namespace sms {
class SMS;
}
}

namespace eyeline {
namespace informer {

class StoreJournal;
class DeliveryImpl;
class DeliveryInfo;
class RetryPolicy;

/// Working storage for messages for one Delivery/Region
class RegionalStorage : protected TransferRequester
{
private:
    friend class EmbedRefPtr<RegionalStorage>;

    /// this one is used to increase granularity, i.e. to lower the lock contention
    static const unsigned CONDITION_COUNT = 13;

    /// ALL messages are kept in this list
    /// The message may be locked from modification for the time of serialization.
    typedef std::multimap< msgtime_type, MsgIter >      ResendQueue;
    typedef smsc::core::buffers::IntHash64< MsgIter >   MessageHash;
    typedef smsc::core::buffers::CyclicQueue< MsgIter > NewQueue;

    /// message locking guard fwd decl
    class MsgLock;

public:

    RegionalStorage( DeliveryImpl&       dlv,
                     regionid_type       regionId );

    virtual ~RegionalStorage();
    dlvid_type getDlvId() const;
    virtual regionid_type getRegionId() const { return regionId_; }
    const DeliveryInfo& getDlvInfo() const;
    DlvState getState() const;

    /// return true if there is no messages in new and resend
    bool isFinished();

    // get the message with given messageid.
    // this method accesses active messages only.
    // bool getMessage( msgid_type msgId, Message& msg );

    /// get the next message.
    /// this method accesses new/resent messages only.
    /// SIDEEFFECT: request more messages into the new queue if needed.
    /// @return true if the message is received.
    bool getNextMessage( msgtime_type currentTime, Message& msg );

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


    /// notify transfer thread that it should be stopped.
    void stopTransfer( bool finalizeAll = false );

    /// rolling over the storage
    /// @return number of bytes written
    size_t rollOver();

    /// invoked at init stage ONLY!
    void setRecordAtInit( Message& msg, regionid_type serial );
    void setNextResendAtInit( msgtime_type nextResend ) {
        nextResendFile_ = nextResend;
    }

    /// invoked after opstore is loaded.
    /// messages accumulated in messageHash_ will be partly moved according to their state
    /// into newQueue and resendQueue.
    bool postInit();

    /// evaluate a number of chunks, fills sms fields
    unsigned evaluateNchunks( const char*     out,
                              size_t          outLen,
                              smsc::sms::SMS* sms = 0 ) const;

protected:
    /// invoked when upload task has finished.
    virtual void transferFinished( InputTransferTask* );

    /// invoked when upload task has finished.
    virtual void transferFinished( ResendTransferTask* );

    /// add new messages to processing.
    virtual void addNewMessages( msgtime_type currentTime,
                                 MessageList& listFrom,
                                 MsgIter      iter1,
                                 MsgIter      iter2 );

    virtual void resendIO( bool isInputDirection );

    /// NOTE: mg must be locked!
    void doFinalize( RelockMutexGuard& mg,
                     MsgIter           iter,
                     msgtime_type      currentTime,
                     uint8_t           state,
                     int               smppState,
                     unsigned          nchunks );

private:
    // message cleanup
    // void destroy( Message& msg );

    inline smsc::core::synchronization::Condition& getCnd( MsgIter iter ) {
        return conds_[unsigned(reinterpret_cast<uint64_t>(reinterpret_cast<const void*>(&(*iter))) / 7) % CONDITION_COUNT];
    }
    void ref() {
        smsc::core::synchronization::MutexGuard mg(refLock_);
        ++ref_;
    }
    void unref() {
        {
            smsc::core::synchronization::MutexGuard mg(refLock_);
            if (ref_>1) {
                --ref_;
                return;
            }
        }
        delete this;
    }

private:
    smsc::logger::Logger*                     log_;
    smsc::core::synchronization::EventMonitor cacheMon_;

    // the list of all messages
    MessageList                               messageList_;

    // iterator that points to item that will be stored automatically from rollOver()
    MsgIter                                   storingIter_;

    /// The message which is stored in the message list
    /// is pointed to from ONLY ONE of messageHash, resendQueue or newQueue!
    /// e.g. they are exclusive.
    MessageHash                       messageHash_;
    ResendQueue                       resendQueue_;
    NewQueue                          newQueue_;

    DeliveryImpl&                     dlv_;
    InputTransferTask*                inputTransferTask_;  // owned
    ResendTransferTask*               resendTransferTask_;  // owned
    regionid_type                     regionId_;

    smsc::core::synchronization::Mutex refLock_;
    unsigned                           ref_;
    unsigned                           newOrResend_; // if <3 then new, otherwise resend

    /// the next resend file starting time or 0 (if there is no files).
    msgtime_type                       nextResendFile_;

    smsc::core::synchronization::Condition conds_[CONDITION_COUNT];
};

typedef EmbedRefPtr< RegionalStorage > RegionalStoragePtr;

}
}

#endif /* !_SCAG_PROTOTYPES_INFOSME_MESSAGEFILE_H */
