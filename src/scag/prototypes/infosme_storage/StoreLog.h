#ifndef _SCAG_PROTOTYPES_INFOSME_STORELOG_H
#define _SCAG_PROTOTYPES_INFOSME_STORELOG_H

#include "Message.h"

namespace scag2 {
namespace prototypes {
namespace infosme {

class StoreLog
{
public:
    StoreLog( const char* fpath );
    ~StoreLog();

    void writeMessage( taskid_type taskId,
                       regionid_type regionId,
                       Message& msg );
private:
    int fd_;
    int version_;
};

}
}
}

#endif /* !_SCAG_PROTOTYPES_INFOSME_STORELOG_H */
