#include "Connector.h"
#include "Message.h"
#include "ProtoException.h"

namespace {
smsc::core::synchronization::Mutex connIdLock;
}

namespace scag2 {
namespace prototypes {
namespace infosme {

unsigned Region::getNextId()
{
    MutexGuard mg(connIdLock);
    static unsigned id = 0;
    return id++;
}


int Region::send( unsigned deltaTime, Message& msg )
{
    // making additional failures
    uint64_t r = random_.getNextNumber();
    int res;
    if ( random_.uniform(1000,r) < 2 ) {
        // stopping it for a second
        suspend(deltaTime+1000);
        res = MessageState::FAIL;
    } else {
        ++sent_;
        speed_.consumeQuant();
        res = MessageState::OK;
    }
    smsc_log_debug(log_,"msg %u %s, %s", msg.getId(),
                   res == MessageState::OK ? "sent" : "failed", toString().c_str());
    return res;
}


void Region::suspend( unsigned deltaTime )
{
    speed_.suspend(deltaTime);
    smsc_log_debug(log_,"suspended, %s", toString().c_str() );
}

}
}
}
