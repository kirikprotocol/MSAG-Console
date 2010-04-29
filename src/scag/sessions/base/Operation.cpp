#include "Operation.h"
#include "Session2.h"
#include "scag/exc/SCAGExceptions.h"

namespace {
scag2::re::RuleStatus defaultRuleStatus;
}

namespace scag2 {
namespace sessions {

using namespace scag::exceptions;
using namespace util::io;

smsc::logger::Logger* Operation::log_ = 0;
Mutex Operation::loggerMutex_;

struct Operation::Segmentation
{
    Segmentation() : sarmr(-1) {}
    ~Segmentation() {}

    int            sarmr;
    re::RuleStatus status;
};


Operation::Operation( Session* s, uint8_t tp ) :
owner_(s),
receivedParts_(0),
receivedResps_(0),
status_(OPERATION_INITED),
type_(tp),
flags_(0),
ctxid_(0),
umr_(-1),
keywords_(0),
segmentation_(0)
{
    if ( !log_ ) {
        MutexGuard mg(loggerMutex_);
        if ( !log_ ) log_ = smsc::logger::Logger::getInstance("sess.op");
    }
}


Operation::~Operation()
{
    if (keywords_) delete keywords_;
    if (segmentation_) delete segmentation_;
}


void Operation::receiveNewPart( int curidx, int lastidx )
{
    if ( curidx < 0 || lastidx < 0 || curidx > lastidx )
        throw SCAGException( "Error: Invalid SMS index (curidx=%d, lastidx=%d)", curidx, lastidx);

    if (lastidx == 0) {
        setFlag( OperationFlags::RECEIVED_ALL_PARTS );
        status_ = OPERATION_INITED;
    } else {
        if ( !receivedParts_ ) {
            status_ = OPERATION_INITED;
        } else {
            status_ = OPERATION_CONTINUED;
        }
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


void Operation::setUSSDref( int32_t ref ) /* throw (SCAGException) */
{
    if ( ref == -1 || (ref == 0 && !flagSet(OperationFlags::SERVICE_INITIATED_USSD_DIALOG) ) ) {
        throw SCAGException( "session=%p/%s setUSSDref(ref=%d), ref should be !=-1, or (=0 && servInitDialog)",
                             owner_, owner_ ? owner_->sessionKey().toString().c_str() : "",
                             ref );
    }
    // changing umr is allowed
    // if ( umr_ > 0 ) throw SCAGException( "setUSSDref(ref=%d), UMR=%d is already set", ref, umr_ );
    // if ( umr_ == -1 ) throw SCAGException( "session=%p/%s setUSSDref(ref=%d), no USSD operation found",
    // this, sessionKey().toString().c_str(), ref );
    umr_ = ref;
}


int32_t Operation::getSARref() const
{
    return segmentation_ ? segmentation_->sarmr : -1;
}


void Operation::setSARref( int32_t sar )
{
    if (!segmentation_) { segmentation_ = new Segmentation; }
    segmentation_->sarmr = sar;
}


const re::RuleStatus& Operation::getSARstatus() const
{
    return segmentation_ ? segmentation_->status : defaultRuleStatus;
}


void Operation::setSARstatus( const re::RuleStatus& st )
{
    if (!segmentation_) { segmentation_ = new Segmentation; }
    segmentation_->status = st;
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
    p.print( "op=%p session=%p opid=%u type=%d(%s) part/resp=%d%s/%d%s umr=%d ctx=%d stat=%s%s%s%s",
             this, owner_, opid, type_, commandOpName(type_),
             receivedParts_, flagSet( OperationFlags::RECEIVED_ALL_PARTS) ? "(ALL)" : "",
             receivedResps_, flagSet( OperationFlags::RECEIVED_ALL_RESPS) ? "(ALL)" : "",
             int(umr_),
             int(ctxid_),
             getNamedStatus(),
             flagSet(OperationFlags::SERVICE_INITIATED_USSD_DIALOG) ? " svcinit" : "",
             // flagSet(OperationFlags::WAIT_RECEIPT) ? " waitrcpt" : "",
             flagSet(OperationFlags::PERSISTENT) ? " pers" : "",
             keywords_ ? " w/kw" : ""
             );
}


Serializer& Operation::serialize( Serializer& s ) const
{
    s << type_ <<
        uint32_t(receivedParts_) <<
        uint32_t(receivedResps_) <<
        uint32_t(ctxid_);
    if ( type_ == CO_USSD_DIALOG ) s << uint32_t(umr_);
    s << uint8_t(status_) <<
        flags_ << ( keywords_ ? *keywords_ : std::string() );
    do {
        if ( s.version() <= 1 ) { break; }

        if ( segmentation_ && segmentation_->sarmr != -1 ) {
            // sar/udh support
            s << uint32_t(segmentation_->sarmr) <<
                uint8_t(segmentation_->status.status) <<
                uint32_t(segmentation_->status.result);
                
        } else {
            s << uint32_t(-1);
        }

    } while ( false );
    return s;
}


Deserializer& Operation::deserialize( Deserializer& s ) /* throw ( DeserializerException ) */
{
    uint32_t x;
    uint8_t y;
    s >> type_;
    s >> x; receivedParts_ = int(x);
    s >> x; receivedResps_ = int(x);
    s >> x; ctxid_ = int32_t(x);
    if ( type_ == CO_USSD_DIALOG ) s >> x; umr_ = int32_t(x);
    s >> y; status_ = ICCOperationStatus(y);
    s >> flags_;
    std::string kw;
    s >> kw;
    setKeywords( kw );
    do {
        if ( s.version() <= 1 ) { break; }
        
        int32_t sarmr = -1;
        s >> sarmr;
        if ( sarmr != -1 ) {
            // sar support
            uint8_t stat;
            uint32_t res;
            s >> stat >> res;
            if (!segmentation_) { segmentation_ = new Segmentation; }
            segmentation_->sarmr = sarmr;
            segmentation_->status.status = re::StatusEnum(stat);
            segmentation_->status.result = res;
        }
        
    } while ( false );
    return s;
}


} // namespace sessions
} // namespace scag
