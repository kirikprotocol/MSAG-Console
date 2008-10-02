#include "Operation.h"
#include "Session2.h"
#include "scag/exc/SCAGExceptions.h"

namespace scag2 {
namespace sessions {

using namespace scag::exceptions;
using namespace util::storage;

smsc::logger::Logger* Operation::log_ = 0;
Mutex Operation::loggerMutex_;


Operation::Operation( Session* s, uint8_t tp ) :
owner_(s),
receivedParts_(0),
receivedResps_(0),
status_(OPERATION_INITED),
type_(tp),
flags_(0),
ctxid_(0)
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

    if (lastidx == 0) {
        setFlag( OperationFlags::RECEIVED_ALL_PARTS );
    } else {
        ++receivedParts_;
        if ( lastidx <= receivedParts_ ) setFlag( OperationFlags::RECEIVED_ALL_PARTS );
    }

    if ( (flags() & OperationFlags::RECEIVED_ALL) == OperationFlags::RECEIVED_ALL ) 
        status_ = OPERATION_COMPLETED;
}


void Operation::receiveNewResp( int curidx, int lastidx )
{
    if ( curidx < 0 || lastidx < 0 || curidx > lastidx )
        throw SCAGException( "Error: Invalid SMS index (curidx=%d, lastidx=%d)", curidx, lastidx);

    status_ = OPERATION_CONTINUED;

    if (lastidx == 0) {
        setFlag( OperationFlags::RECEIVED_ALL_RESPS );
    } else {
        ++receivedResps_;
        if ( lastidx <= receivedResps_ ) setFlag( OperationFlags::RECEIVED_ALL_RESPS );
    }

    if ( (flags() & OperationFlags::RECEIVED_ALL) == OperationFlags::RECEIVED_ALL ) 
        status_ = OPERATION_COMPLETED;
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
        return "???";
    };
}

void Operation::print( util::Print& p, opid_type opid ) const
{
    if ( ! p.enabled() ) return;
    p.print( "op=%p session=%p opid=%u type=%d(%s) part/resp=%d%s/%d%s ctx=%d stat=%s%s%s%s",
             this, owner_, opid, type_, commandOpName(type_),
             receivedParts_, flagSet( OperationFlags::RECEIVED_ALL_PARTS) ? "(ALL)" : "",
             receivedResps_, flagSet( OperationFlags::RECEIVED_ALL_RESPS) ? "(ALL)" : "",
             int(ctxid_),
             getNamedStatus(),
             flagSet(OperationFlags::SERVICE_INITIATED_USSD_DIALOG) ? " svcinit" : "",
             flagSet(OperationFlags::WAIT_RECEIPT) ? " waitrcpt" : "",
             flagSet(OperationFlags::PERSISTENT) ? " pers" : ""
             );
}


Serializer& Operation::serialize( Serializer& s ) const
{
    return s << 
        uint32_t(receivedParts_) <<
        uint32_t(receivedResps_) <<
        uint32_t(ctxid_) <<
        uint8_t(status_) <<
        type_ << flags_;
}


Deserializer& Operation::deserialize( Deserializer& s ) throw ( DeserializerException )
{
    uint32_t x;
    s >> x; receivedParts_ = int(x);
    s >> x; receivedResps_ = int(x);
    s >> x; ctxid_ = int32_t(x);
    uint8_t y;
    s >> y; status_ = ICCOperationStatus(y);
    s >> type_ >> flags_;
    return s;
}


} // namespace sessions
} // namespace scag
