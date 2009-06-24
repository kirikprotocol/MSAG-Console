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
    virtual ~Response() {}
    virtual bool isRequest() const { return false; }
    uint8_t getStatus() const { return status_; }
    void setStatus( uint8_t status ) { status_ = status; }
    virtual const char* typeToString() const = 0;
    virtual bool visit( ResponseVisitor& visitor ) = 0;

private:
    uint8_t status_;
};

} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_REQUEST_H */
