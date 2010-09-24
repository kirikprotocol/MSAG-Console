#ifndef _INFORMER_INPUTMESSAGESOURCE_H
#define _INFORMER_INPUTMESSAGESOURCE_H

#include "Message.h"

namespace eyeline {
namespace informer {

class InputMessageUploadRequester
{
public:    
    virtual ~InputMessageUploadRequester() {}

    /// identification
    virtual dlvid_type getDlvId() const = 0;
    virtual regionid_type getRegionId() const = 0;

    /// notify that current upload is finished
    virtual void uploadFinished() = 0;

    /// add new messages
    virtual void addNewMessages( msgtime_type currentTime,
                                 MessageList& listFrom,
                                 MsgIter iter1,
                                 MsgIter iter2 ) = 0;
};


/// an interface for an input message source
class InputMessageSource
{
public:
    virtual ~InputMessageSource() {}

    /// request 'count' messages to be uploaded for 'requester'.
    /// This method should create a ThreadedTask which will upload messages,
    /// or if the task is already running, simply return.
    /// NOTE: throws exception if the task cannot be created/found.
    virtual void requestInputMessages( InputMessageUploadRequester& requester,
                                       unsigned count ) = 0;
};

} // informer
} // smsc

#endif
