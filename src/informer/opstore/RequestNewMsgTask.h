#ifndef _INFORMER_REQUESTNEWMSGTASK_H
#define _INFORMER_REQUESTNEWMSGTASK_H

#include "informer/data/InputMessageSource.h"
#include "core/threads/ThreadedTask.hpp"

namespace eyeline {
namespace informer {

class InputMessageUploadRequester;

class RequestNewMsgTask : public smsc::core::threads::ThreadedTask
{
protected:
    RequestNewMsgTask( InputMessageUploadRequester& req, unsigned count ) :
    req_(&req), count_(count) {}

    virtual void onRelease();
    /*
        smsc::core::threads::ThreadedTask::onRelease();
        InputMessageRequester* r = req_;
        if (r) r->uploadFinished();
    }
     */

protected:
    InputMessageUploadRequester* req_; // not owned
    unsigned                     count_;
};

} // informer
} // smsc

#endif
