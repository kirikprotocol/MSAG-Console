#ifndef _INFORMER_REGIONALSTORAGE_H
#define _INFORMER_REGIONALSTORAGE_H

#include <map>
#include "core/buffers/CyclicQueue.hpp"
#include "core/buffers/IntHash.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/synchronization/Condition.hpp"
#include "informer/io/EmbedRefPtr.h"
#include "informer/data/InputMessageSource.h"
#include "logger/Logger.h"

namespace eyeline {
namespace informer {

class StoreJournal;
class DeliveryImpl;
class DeliveryInfo;

/// Working storage for messages for one Delivery/Region
class RegionalStorage : public TransferRequester
{
private:
    friend class EmbedRefPtr<RegionalStorage>;

    /// this one is used to increase granularity, i.e. to lower the lock contention
    static const unsigned CONDITION_COUNT = 13;

    /// ALL messages are kept in this list
    /// The message may be locked from modification for the time of serialization.
    typedef std::multimap< msgtime_type, MsgIter >      ResendQueue;
    typedef smsc::core::buffers::IntHash< MsgIter >     MessageHash;
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
    void retryMessage( msgid_type msgId,
                       msgtime_type currentTime,
                       int smppState );

    /// finalize message. message is removed from cache.
    void finalizeMessage( msgid_type   msgId,
                          msgtime_type currentTime,
                          uint8_t      state,
                          int          smppState );


    /// invoked when upload task has finished.
    virtual void transferFinished( TransferTask* );

    /// notify transfer thread that it should be stopped.
    void stopTransfer( bool finalizeAll = false );

    /// add new messages to processing.
    virtual void addNewMessages( msgtime_type currentTime,
                                 MessageList& listFrom,
                                 MsgIter      iter1,
                                 MsgIter      iter2 );

    /// rolling over the storage
    /// @return number of bytes written
    size_t rollOver();

    /// invoked at init stage ONLY!
    void setRecordAtInit( Message& msg, regionid_type serial );

    /// invoked after opstore is loaded.
    /// messages accumulated in messageHash_ will be partly moved according to their state
    /// into newQueue and resendQueue.
    bool postInit();

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
        smsc::core::synchronization::MutexGuard mg(refLock_);
        if (ref_<=1) {
            delete this;
        } else {
            --ref_;
        }
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
    TransferTask*                     transferTask_;  // owned
    regionid_type                     regionId_;

    smsc::core::synchronization::Mutex refLock_;
    unsigned                           ref_;

    smsc::core::synchronization::Condition conds_[CONDITION_COUNT];
};

typedef EmbedRefPtr< RegionalStorage > RegionalStoragePtr;

}
}

#endif /* !_SCAG_PROTOTYPES_INFOSME_MESSAGEFILE_H */
