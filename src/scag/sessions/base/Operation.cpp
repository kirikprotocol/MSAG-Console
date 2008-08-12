#include "Operation.h"
#include "Session2.h"
#include "scag/exc/SCAGExceptions.h"

namespace scag2 {
namespace sessions {

using namespace scag::exceptions;

smsc::logger::Logger* Operation::log_ = 0;
Mutex Operation::loggerMutex_;


Operation::Operation( Session* s, uint8_t tp ) :
owner_(s),
receivedParts_(0),
receivedAllParts_(false),
receivedResps_(0),
receivedAllResps_(false),
status_(OPERATION_INITED),
type_(tp),
flags_(0)
{
    if ( !log_ ) {
        MutexGuard mg(loggerMutex_);
        if ( !log_ ) log_ = smsc::logger::Logger::getInstance("sess.op");
    }
}


void Operation::receiveNewPart( int curidx, int lastidx )
{
    if ( curidx < 0 || lastidx < 0 || curidx > lastidx )
        throw SCAGException( "Error: Invalid SMS index (curidx=%d, lastidx=%d)", curidx, lastidx);

    if ( curidx == 0 )
        status_ = OPERATION_INITED;
    else
        status_ = OPERATION_CONTINUED;

    if (lastidx == 0)
    {
        receivedAllParts_ = true;
    } else {
        ++receivedParts_;
        receivedAllParts_ = (lastidx <= receivedParts_);
    }

    if ( receivedAllResps_ && receivedAllParts_ ) status_ = OPERATION_COMPLETED;
}


void Operation::receiveNewResp( int curidx, int lastidx )
{
    //smsc_log_debug(logger,"Operation: Change status resp (CI=%d, LI=%d, allParts=%d)",currentIndex, lastIndex, m_receivedAllParts);

    if ( curidx < 0 || lastidx < 0 || curidx > lastidx )
        throw SCAGException( "Error: Invalid SMS index (curidx=%d, lastidx=%d)", curidx, lastidx);

    status_ = OPERATION_CONTINUED;

    if ( lastidx == 0 )
    {
        receivedAllResps_ = true;
    } else {
        ++receivedResps_;
        receivedAllResps_ = ( lastidx <= receivedResps_ );
    }

    if ( receivedAllResps_ && receivedAllParts_ ) status_ = OPERATION_COMPLETED;
}


const char* Operation::getNamedStatus() const
{
    switch (status_) {
    case OPERATION_INITED :
        return "INITED";
    case OPERATION_CONTINUED :
        return "CONTINUED";
    case OPERATION_COMPLETED :
        return "COMPLETED";
    default:
        return "UNKNOWN";
    };
}

} // namespace sessions
} // namespace scag
