#ifndef _SCAG_PROTOTYPES_INFOSME_CONNECTOR_H
#define _SCAG_PROTOTYPES_INFOSME_CONNECTOR_H

#include "logger/Logger.h"
#include "scag/util/io/Drndm.h"

namespace scag2 {
namespace prototypes {
namespace infosme {

class Message;

class Connector
{
private:
    static unsigned getNextId();

public:
    Connector( unsigned bandwidth ) :
    id_(getNextId()),
    log_(0),
    bandwidth_(bandwidth),
    wouldSend_(0),
    nextTime_(0),
    sent_(0)
    {
        char buf[40];
        sprintf(buf,"conn.%03u",id_);
        name_ = buf;
        log_ = smsc::logger::Logger::getInstance(name_.c_str());
        random_.setSeed(time(0));
    }

    int send( unsigned deltaTime, Message& msg );
    void suspend( unsigned deltaTime );

    unsigned getId() const { return id_; }

    /// check if the connector is ready.
    /// @return 0 -- connector is ready, >0 how many ms to wait until it is.
    unsigned isReady( unsigned deltaTime );

    std::string toString() const {
        char buf[256];
        std::sprintf(buf,"%s band=%u wds=%u sent=%u next=%u",name_.c_str(),bandwidth_,wouldSend_,sent_,nextTime_);
        return buf;
    }

    bool operator < ( const Connector& other ) const {
        if ( nextTime_ < other.nextTime_ ) return true;
        if ( nextTime_ > other.nextTime_ ) return false;
        if ( bandwidth_ > other.bandwidth_ ) return true;
        if ( bandwidth_ < other.bandwidth_ ) return false;
        if ( id_ < other.id_ ) return true;
        return false;
    }

private:
    unsigned id_;
    smsc::logger::Logger* log_;
    std::string name_;
    unsigned bandwidth_;    // msg per sec
    unsigned wouldSend_;    // msg * 1000 since start
    unsigned nextTime_;
    unsigned sent_;         // real number of send msgs
    scag2::util::Drndm random_;
};

}
}
}

#endif
