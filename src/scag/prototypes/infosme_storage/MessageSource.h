#ifndef _SCAG_PROTOTYPES_INFOSME_MESSAGESOURCE_H
#define _SCAG_PROTOTYPES_INFOSME_MESSAGESOURCE_H

#include "Message.h"
#include "core/threads/ThreadedTask.hpp"

namespace scag2 {
namespace prototypes {
namespace infosme {

class MessageUploadTask;

class MessageUploadRequester
{
public:
    virtual ~MessageUploadRequester() {}
    virtual void uploadFinished() = 0;
    virtual void addNewMessages( msgtime_type currentTime,
                                 MessageList& listFrom,
                                 MsgIter      iter1,
                                 MsgIter      iter2 ) = 0;
};


class MessageUploadTask : public smsc::core::threads::ThreadedTask
{
protected:
    MessageUploadTask( MessageUploadRequester& requester ) :
    smsc::core::threads::ThreadedTask(), requester_(&requester) {}

    virtual ~MessageUploadTask() {
        requester_->uploadFinished();
    }
    virtual const char* taskName() { return "msgUploadTask"; }

protected:
    MessageUploadRequester* requester_;
};



/// an interface for a message source (typically it will be a storage of new messages)
class MessageSource
{
public:
    virtual ~MessageSource() {}

    // return a message upload task, the task is already inserted into message upload pool
    // NOTE: TODO: only throw exception if the MessageUploadTask cannot be created.
    virtual void requestNewMessages( MessageUploadRequester& requester,
                                     taskid_type             taskId,
                                     regionid_type           regionId,
                                     unsigned                count ) = 0;
};

}
}
}

#endif /* !_SCAG_PROTOTYPES_INFOSME_MESSAGEFILE_H */
