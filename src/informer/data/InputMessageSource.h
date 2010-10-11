#ifndef _INFORMER_INPUTMESSAGESOURCE_H
#define _INFORMER_INPUTMESSAGESOURCE_H

#include "informer/data/Message.h"
#include "core/threads/ThreadedTask.hpp"

namespace eyeline {
namespace informer {

class TransferTask;
class MessageGlossary;
struct InputRegionRecord;

/// a class requesting transfer of input messages
/// from instore into opstore.
class TransferRequester
{
public:    
    virtual ~TransferRequester() {}

    // identification
    // virtual dlvid_type getDlvId() const = 0;
    virtual regionid_type getRegionId() const = 0;

    /// notify that current upload is finished
    virtual void transferFinished( TransferTask* task ) = 0;

    /// add new messages
    virtual void addNewMessages( msgtime_type currentTime,
                                 MessageList& listFrom,
                                 MsgIter iter1,
                                 MsgIter iter2 ) = 0;
};


class TransferTask : public smsc::core::threads::ThreadedTask
{
public:
    virtual ~TransferTask() {}
protected:
    TransferTask( TransferRequester& req, unsigned count ) :
    smsc::core::threads::ThreadedTask(false),
    requester_(req), count_(count) {}

    virtual void onRelease() {
        smsc::core::threads::ThreadedTask::onRelease();
        TransferRequester* r = &requester_;
        if (r) r->transferFinished(this);
    }

private:
    TransferTask( const TransferTask& );

protected:
    TransferRequester& requester_;
    unsigned count_;
};


/// an interface for an input message source
class InputMessageSource
{
public:
    virtual ~InputMessageSource() {}

    /// add new messages, the list of messages is modified
    /// and will contain msgids.
    virtual void addNewMessages( MsgIter begin, MsgIter end ) = 0;

    // request 'count' messages to be uploaded for 'requester'.
    // This method should create a ThreadedTask which will upload messages,
    // or if the task is already running, simply return.
    // NOTE: throws exception if the task cannot be created/found.
    virtual TransferTask* startTransferTask( TransferRequester& requester,
                                             unsigned           count,
                                             bool mayDetachRegion ) = 0;

    virtual MessageGlossary& getGlossary() = 0;

    virtual dlvid_type getDlvId() const = 0;
    virtual const std::string& getStorePath() const = 0;

    /// NOTE: this method is invoked at init ONLY!
    virtual void setRecordAtInit( regionid_type            regionId,
                                  const InputRegionRecord& rec,
                                  uint64_t                 maxMsgId ) = 0;
};

} // informer
} // smsc

#endif
