#ifndef SCAG_BILL_EWALLET_RESPONSE_H
#define SCAG_BILL_EWALLET_RESPONSE_H

#include "Packet.h"
#include "Status.h"

namespace scag2 {
namespace bill {
namespace ewallet {

class ResponseVisitor;

class Response: public Packet
{
public:
    Response( uint8_t status = Status::UNKNOWN ) : status_(status) {}
    virtual ~Response() {
        CHECKMAGTC;
    }
    virtual bool isRequest() const { return false; }
    virtual std::string toString() const {
        CHECKMAGTC;
        char buf[50];
        snprintf(buf,sizeof(buf),"%s status=%u/%s",
                 Packet::toString().c_str(),unsigned(status_),Status::statusToString(status_));
        return buf;
    }
    uint8_t getStatus() const { return status_; }
    void setStatus( uint8_t status ) {
        CHECKMAGTC;
        status_ = status; 
    }
    virtual const char* typeToString() const = 0;
    virtual bool visit( ResponseVisitor& visitor ) = 0;

private:
    uint8_t status_;
};

} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_REQUEST_H */
