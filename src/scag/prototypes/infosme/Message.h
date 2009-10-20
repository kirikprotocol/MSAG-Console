#ifndef _SCAG_PROTOTYPES_INFOSME_MESSAGE_H
#define _SCAG_PROTOTYPES_INFOSME_MESSAGE_H

#include <time.h>

namespace scag2 {
namespace prototypes {
namespace infosme {

class Message
{
private:
    static unsigned getNextId();

public:
    Message() {}
    Message(unsigned regionId, time_t start) :
    id_(getNextId()), regionId_(regionId), start_(start) {}

    unsigned getId() const { return id_; }
    unsigned getRegionId() const { return regionId_; }
    time_t startTime() const { return start_; }

    bool operator < ( const Message& other ) const {
        if ( start_ < other.start_ ) return true;
        if ( start_ > other.start_ ) return false;
        if ( id_ < other.id_ ) return true;
        return false;
    }

private:
    unsigned    id_;
    unsigned    regionId_;
    time_t start_;
};

}
}
}

#endif
