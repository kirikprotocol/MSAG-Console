#ifndef _SCAG_PROTOTYPES_INFOSME_CONNECTOR_H
#define _SCAG_PROTOTYPES_INFOSME_CONNECTOR_H

#include "logger/Logger.h"
#include "scag/util/io/Drndm.h"
#include "Speed.h"

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
    speed_(bandwidth),
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

    inline unsigned getId() const { return id_; }
    inline unsigned getBandwidth() const { return speed_.getSpeed(); }

    /// check if the connector is ready.
    /// @return 0 -- connector is ready, >0 how many ms to wait until it is.
    inline unsigned isReady( unsigned deltaTime ) { return speed_.isReady(deltaTime); }

    std::string toString() const {
        char buf[256];
        std::sprintf(buf,"%s band=%u sent=%u next=%u",
                     name_.c_str(),speed_.getSpeed(),sent_,speed_.getNextTime());
        return buf;
    }

    bool operator < ( const Connector& other ) const {
        const int cmp = speed_.compare( other.speed_ );
        if ( cmp < 0 ) return true;
        if ( cmp > 0 ) return false;
        if ( id_ < other.id_ ) return true;
        return false;
    }

private:
    unsigned id_;
    smsc::logger::Logger* log_;
    std::string name_;
    Speed    speed_;
    unsigned sent_;         // real number of send msgs
    scag2::util::Drndm random_;
};

}
}
}

#endif
