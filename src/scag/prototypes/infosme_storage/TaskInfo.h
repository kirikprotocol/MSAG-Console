#ifndef _SCAG_PROTOTYPES_INFOSME_TASKINFO_H
#define _SCAG_PROTOTYPES_INFOSME_TASKINFO_H

#include "Message.h"

namespace scag2 {
namespace prototypes {
namespace infosme {

class DlvInfo
{
public:
    DlvInfo( dlvid_type dlvid ) : dlvid_(dlvid) {}

    dlvid_type getDlvId() const { return dlvid_; }

    unsigned getPriority() const { return 1; }

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
    dlvid_type dlvid_;
};

}
}
}

#endif /* !_SCAG_PROTOTYPES_INFOSME_TASKINFO_H */
