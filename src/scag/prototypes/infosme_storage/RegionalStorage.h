#ifndef _SCAG_PROTOTYPES_INFOSME_REGIONALSTORAGE_H
#define _SCAG_PROTOTYPES_INFOSME_REGIONALSTORAGE_H

#include <list>
#include <map>
#include "Message.h"
#include "TaskInfo.h"
#include "MessageSource.h"
#include "logger/Logger.h"
#include "core/buffers/IntHash.hpp"
#include "core/buffers/CyclicQueue.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/Condition.hpp"

namespace scag2 {
namespace prototypes {
namespace infosme {

class StoreLog;
class RegionalStoragePtr;

class RegionalStorage : public MessageUploadRequester
{
private:
    friend class RegionalStoragePtr;

    static const unsigned CONDITION_COUNT = 13;

    /// ALL messages are kept in this list
    /// The message may be locked from modification for the time of serialization.
    // typedef std::list< MessageData >                    MessageList;
    // typedef MessageList::iterator                       MsgIter;
    typedef std::multimap< msgtime_type, MsgIter >      ResendQueue;
    typedef smsc::core::buffers::IntHash< MsgIter >     MessageHash;
    typedef smsc::core::buffers::CyclicQueue< MsgIter > NewQueue;

    class MsgLock;

public:

    RegionalStorage( const TaskInfo& taskInfo,
                     regionid_type   regionId,
                     StoreLog&       storeLog,
                     MessageSource&  messageSource );


    ~RegionalStorage();


    regionid_type getRegionId() const { return regionId_; }
    const TaskInfo& getTaskInfo() const { return *taskInfo_; }
    taskid_type getTaskId() const { return taskInfo_->getTaskId(); }

    /// get the message with given messageid.
    /// this method accesses active messages only.
    bool getMessage( msgid_type msgId, Message& msg );

    /// get the next message.
    /// this method accesses new/resent messages only.
    /// SIDEEFFECT: request more messages into the new queue if needed.
    /// @return if the message is received.
    bool getNextMessage( msgtime_type currentTime, Message& msg );

    /// change message state when receipt has come.
    /// message is left in the cache.
    void messageSent( msgid_type msgId,
                      msgtime_type currentTime );

    /// change message state when temporal failure received.
    /// message is removed from the cache.
    void retryMessage( msgid_type msgId,
                       msgtime_type currentTime,
                       msgtime_type retryTime,
                       int smppState );

    /// finalize message. message is removed from cache.
    void finalizeMessage( msgid_type   msgId,
                          msgtime_type currentTime,
                          uint8_t      state,
                          int          smppState );

    /// add new messages to processing.
    virtual void addNewMessages( msgtime_type currentTime,
                                 MessageList& listFrom,
                                 MsgIter      iter1,
                                 MsgIter      iter2 );

    /// rolling over the storage
    void rollOver();

    /// invoked when upload task has finished.
    virtual void uploadFinished();

private:
    /// message cleanup
    void destroy( Message& msg );
    inline smsc::core::synchronization::Condition& getCnd( MsgIter iter ) {
        return conds_[unsigned(reinterpret_cast<uint64_t>(reinterpret_cast<const void*>(&(*iter))) / 7) % CONDITION_COUNT];
    }
    void usage( bool incr );

private:
    smsc::logger::Logger*                               log_;
    smsc::core::synchronization::Mutex                  cacheLock_;
    MessageList                                         messageList_;
    // iterator that points to item that will be stored
    MsgIter                                             storingIter_;
    // iterator that points to the beginning of valid items,
    // the items that are before it will be destroyed eventually.
    MsgIter                                             validItems_;

    /// The message which is stored in the message list
    /// is pointed to from ONLY ONE of messageHash, resendQueue or newQueue!
    /// e.g. they are exclusive.
    MessageHash                       messageHash_;
    ResendQueue                       resendQueue_;
    NewQueue                          newQueue_;

    StoreLog*                         alog_;
    MessageSource*                    messageSource_;
    unsigned                          uploadTasks_; // a number of upload tasks

    const TaskInfo*                   taskInfo_;
    regionid_type                     regionId_;
    unsigned                          usage_;

    smsc::core::synchronization::Condition conds_[CONDITION_COUNT];
};


class RegionalStoragePtr
{
public:
    RegionalStoragePtr( RegionalStorage* ptr = 0 ) : ptr_(ptr) {
        if (ptr_) { ptr_->usage(true); }
    }
    RegionalStoragePtr( const RegionalStoragePtr& p ) : ptr_(p.ptr_) {
        if (p.ptr_) { p.ptr_->usage(true); }
    }
    RegionalStoragePtr& operator = ( const RegionalStoragePtr& p ) {
        if ( &p != this ) {
            if (ptr_) { ptr_->usage(false); }
            ptr_ = p.ptr_;
            if (ptr_) { ptr_->usage(true); }
        }
        return *this;
    }
    ~RegionalStoragePtr() {
        if (ptr_) { ptr_->usage(false); }
    }

    RegionalStorage* get() {
        return ptr_;
    }
    RegionalStorage* operator-> () {
        return ptr_;
    }

private:
    RegionalStorage* ptr_;
};

}
}
}

#endif /* !_SCAG_PROTOTYPES_INFOSME_MESSAGEFILE_H */
