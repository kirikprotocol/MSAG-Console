#ifndef SCAG_BILL_EWALLET_PINGRESP_H
#define SCAG_BILL_EWALLET_PINGRESP_H

#include "Response.h"
#include "ResponseVisitor.h"

namespace scag2 {
namespace bill {
namespace ewallet {

class PingResp: public Response
{
public:
    PingResp( uint8_t status = Status::UNKNOWN ) : Response(status) {}
    virtual ~PingResp() {}
    virtual std::string toString() const { return "pingresp"; }
    virtual const char* typeToString() const { return "pingresp"; }
    virtual bool isValid() const { return true; }
    virtual bool visit( ResponseVisitor& visitor ) { return visitor.visitPingResp(*this); }
};

} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_PINGRESP_H */
