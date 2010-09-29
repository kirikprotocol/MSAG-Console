#include "RequestNewMsgTask.h"

namespace eyeline {
namespace informer {

void RequestNewMsgTask::onRelease()
{
    smsc::core::threads::ThreadedTask::onRelease();
    InputMessageUploadRequester* r = req_;
    req_ = 0;
    if (r) { r->uploadFinished(); }
}

}
}
