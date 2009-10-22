#include "Connector.h"
#include "Message.h"
#include "ProtoException.h"

namespace {
smsc::core::synchronization::Mutex connIdLock;
}

namespace scag2 {
namespace prototypes {
namespace infosme {

unsigned Connector::getNextId()
{
    MutexGuard mg(connIdLock);
    static unsigned id = 0;
    return id++;
}


int Connector::send( unsigned deltaTime, Message& msg )
{
    // making additional failures
    uint64_t r = random_.getNextNumber();
    if ( random_.uniform(1000,r) < 2 ) {
        // stopping it for a second
        nextTime_ = deltaTime + 1000;
        wouldSend_ = nextTime_ * bandwidth_;
        smsc_log_debug(log_,"failed to send msg %u: %s",msg.getId(),toString().c_str());
        return MessageState::FAIL;
    }
    ++sent_;
    wouldSend_ += 1000;
    nextTime_ = wouldSend_ / bandwidth_;
    if ( nextTime_+1000 < deltaTime ) {
        nextTime_ = deltaTime;
        wouldSend_ = nextTime_*bandwidth_;
    } else if ( nextTime_ > deltaTime+1000 ) {
        nextTime_ = deltaTime+1000;
        wouldSend_ = nextTime_*bandwidth_;
    }
    smsc_log_debug(log_,"msg %u sent, %s", msg.getId(), toString().c_str());
    return MessageState::OK;
}


void Connector::suspend( unsigned deltaTime )
{
    if ( deltaTime > nextTime_ ) {
        nextTime_ = deltaTime;
        wouldSend_ = nextTime_ * bandwidth_;
    }
    smsc_log_debug(log_,"suspended, %s", toString().c_str() );
}


unsigned Connector::isReady( unsigned deltaTime )
{
    if ( deltaTime >= nextTime_ ) return 0;
    return nextTime_ - deltaTime;
}

}
}
}
