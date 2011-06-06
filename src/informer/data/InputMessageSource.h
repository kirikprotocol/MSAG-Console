#ifndef _INFORMER_INPUTMESSAGESOURCE_H
#define _INFORMER_INPUTMESSAGESOURCE_H

#include <vector>
#include "informer/data/Message.h"
#include "informer/io/EmbedRefPtr.h"
#include "core/threads/ThreadedTask.hpp"

namespace eyeline {
namespace informer {

class InputTransferTask;
class ResendTransferTask;
// class MessageGlossary;
struct InputRegionRecord;
class ActivityLog;
class Region;

/// a class requesting transfer of input messages
/// from instore into opstore.
class TransferRequester
{
    friend class EmbedRefPtr< TransferRequester >;
public:
    TransferRequester() : refLock_(MTXWHEREAMI), ref_(0) {}

    virtual ~TransferRequester() {}

    // identification
    // virtual dlvid_type getDlvId() const = 0;
    virtual const Region& getRegion() const = 0;

    /// notify that current upload is finished
    virtual void transferFinished( InputTransferTask* task ) = 0;
    virtual void transferFinished( ResendTransferTask* task ) = 0;

    /// add new messages
    virtual void addNewMessages( msgtime_type currentTime,
                                 MessageList& listFrom ) = 0;

    /// resend file input/output
    virtual void resendIO( bool isInputDirection, volatile bool& stopFlag ) = 0;

protected:
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

protected:
    smsc::core::synchronization::Mutex refLock_;
    unsigned                           ref_;
};


class InputTransferTask : public smsc::core::threads::ThreadedTask
{
public:
    virtual ~InputTransferTask() {}
protected:
    InputTransferTask( TransferRequester& req, unsigned count ) :
    smsc::core::threads::ThreadedTask(false),
    requester_(&req), count_(count) {}

    virtual void onRelease() {
        smsc::core::threads::ThreadedTask::onRelease();
        requester_->transferFinished(this);
        requester_.reset(0);
        delete this;
    }

private:
    InputTransferTask( const InputTransferTask& );

protected:
    EmbedRefPtr<TransferRequester> requester_;
    unsigned count_;
};


class ResendTransferTask : public smsc::core::threads::ThreadedTask
{
public:
    virtual ~ResendTransferTask() {}

    ResendTransferTask( TransferRequester& req, bool isInputDir ) :
    smsc::core::threads::ThreadedTask(false),
    requester_(&req), isInputDir_(isInputDir) {}

protected:
    virtual void onRelease() {
        smsc::core::threads::ThreadedTask::onRelease();
        requester_->transferFinished(this);
        requester_.reset(0);
        delete this;
    }

    // the direction of task (true -- load from disk, false -- flush to disk).
    // inline bool isInput() const { return isInputDir_; }

    virtual const char* taskName() { return "resendIO"; }

    virtual int Execute() {
        requester_->resendIO(isInputDir_,isStopping);
        return 0;
    }

private:
    ResendTransferTask( const ResendTransferTask& );

protected:
    EmbedRefPtr<TransferRequester> requester_;
    bool                           isInputDir_;
};


/// an interface for an input message source
class InputMessageSource
{
public:
    virtual ~InputMessageSource() {}

    /// bind storage with activity log, it is needed to be able to
    /// make records about new messages.
    virtual void init( ActivityLog& actlog ) = 0;

    // virtual DeliveryActivator& getDlvActivator() = 0;

    /// add new messages, the list of messages is modified
    /// and will contain msgids.
    virtual void addNewMessages( MsgIter begin, MsgIter end ) = 0;

    /// mark the messages as dropped.
    virtual void dropMessages( const std::vector<msgid_type>& msgids ) = 0;

    // request 'count' messages to be uploaded for 'requester'.
    // This method should create a ThreadedTask which will upload messages.
    // NOTE: throws exception if the task cannot be created/found.
    virtual InputTransferTask* createInputTransferTask( TransferRequester& requester,
                                                        unsigned           count ) = 0;

    // virtual MessageGlossary& getGlossary() = 0;

    /// NOTE: this method is invoked at init ONLY!
    virtual void setRecordAtInit( const InputRegionRecord& rec,
                                  uint64_t                 maxMsgId ) = 0;

    /// invoked at init stage ONLY!
    /// @a filledRegs at return contains regions that have some messages to send.
    virtual void postInit( std::vector<regionid_type>& filledRegs ) = 0;

    /// rolling over whole storage
    virtual size_t rollOver() = 0;
};

} // informer
} // smsc

#endif
