#ifndef SCAG_BILL_EWALLET_PROTO_CONTEXT_H
#define SCAG_BILL_EWALLET_PROTO_CONTEXT_H

#include <memory>
#include "util/int.h"
#include "util/TypeInfo.h"
#include "logger/Logger.h"
#include "scag/util/Time.h"
#include "scag/bill/ewallet/Request.h"
#include "scag/bill/ewallet/Response.h"

namespace scag2 {
namespace bill {
namespace ewallet {

namespace proto {

class Context
{
public:
    enum ContextState {
            NONE     = 0,
            SENDING  = 1, // packet is fetched from sending queue and is being sending
            RECEIVED = 2, // packet is just received
            SENT     = 3, // packet is successfully sent
            DONE     = 4, // both received and sent
            EXPIRED  = 5, // packet is expired in sending queue
            FAILED   = 6  // sending failed
    };

public:
    Context( Request* request, Response* resp ) :
    creationTime_(util::currentTimeMillis()), req_(request),
    resp_(resp), seqnum_(0), state_(NONE) {
        if ( request ) {
            seqnum_ = request->getSeqNum(); 
            if ( resp && resp->getSeqNum() != seqnum_ ) {
                smsc_log_warn(smsc::logger::Logger::getInstance("ewall"),
                              "different seqnums req=%u resp=%u, fixing", seqnum_, resp->getSeqNum() );
                resp->setSeqNum(seqnum_);
            }
        } else if ( resp ) {
            seqnum_ = resp->getSeqNum();
        }
    }

    virtual ~Context() {
        CHECKMAGTC;
    }

    uint32_t getSeqNum() const { return seqnum_; }
    void setSeqNum( uint32_t seqnum ) {
        CHECKMAGTC;
        seqnum_ = seqnum;
        if ( getRequest().get() ) getRequest()->setSeqNum( seqnum );
        if ( getResponse().get() ) getResponse()->setSeqNum( seqnum );
    }

    util::msectime_type getCreationTime() const {
        CHECKMAGTC;
        return creationTime_;
    }

    std::auto_ptr<Request>&   getRequest() { return req_; }
    std::auto_ptr<Response>&  getResponse() { return resp_; }

    ContextState getState() const { return state_; }
    virtual void setState( ContextState state ) {
        CHECKMAGTC;
        state_ = state;
    }

protected:
    DECLMAGTC(Context);
    util::msectime_type       creationTime_;
private:
    std::auto_ptr< Request >  req_;
    std::auto_ptr< Response > resp_;
    uint32_t                  seqnum_;
    ContextState              state_;
};

} // namespace proto
} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_PROTO_CONTEXT_H */
