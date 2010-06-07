#ifndef _SCAG_PROTOTYPES_INFOSME_STORELOG_H
#define _SCAG_PROTOTYPES_INFOSME_STORELOG_H

#include "logger/Logger.h"
#include "Message.h"

namespace scag2 {
namespace prototypes {
namespace infosme {

class StoreLog
{
public:
    StoreLog( const char* fpath );
    ~StoreLog();

    void writeMessage( dlvid_type dlvId,
                       regionid_type regionId,
                       Message& msg );
private:
    smsc::logger::Logger* log_;
    int fd_;
    int version_;
};

}
}
}

#endif /* !_SCAG_PROTOTYPES_INFOSME_STORELOG_H */
