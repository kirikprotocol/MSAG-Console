#ifndef _SCAG_PROTOTYPES_INFOSME_TASKINFO_H
#define _SCAG_PROTOTYPES_INFOSME_TASKINFO_H

#include "Message.h"

namespace scag2 {
namespace prototypes {
namespace infosme {

class TaskInfo
{
public:
    TaskInfo( taskid_type taskId ) : taskId_(taskId) {}
    taskid_type getTaskId() const { return taskId_; }
    msgtime_type getMinRetryTime() const {
        return msgtime_type(10);
    }
    unsigned getMaxResendQueueSize() const {
        return 100000U;
    }
    unsigned getMinNewQueueSize() const {
        return 3;
    }
    unsigned getUploadCount() const {
        return 5;
    }

private:
    taskid_type taskId_;
};

}
}
}

#endif /* !_SCAG_PROTOTYPES_INFOSME_TASKINFO_H */
