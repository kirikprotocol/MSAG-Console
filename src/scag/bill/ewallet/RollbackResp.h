#ifndef SCAG_BILL_EWALLET_ROLLBACKRESP_H
#define SCAG_BILL_EWALLET_ROLLBACKRESP_H

#include "Response.h"
#include "ResponseVisitor.h"

namespace scag2 {
namespace bill {
namespace ewallet {

class RollbackResp: public Response
{
public:
    RollbackResp( uint8_t status = Status::UNKNOWN ) : Response(status) {}
    virtual ~RollbackResp() {}
    virtual std::string toString() const { return "rollbackresp"; }
    virtual const char* typeToString() const { return "rollbackresp"; }
    virtual bool isValid() const { return true; }
    virtual bool visit( ResponseVisitor& visitor ) { return visitor.visitRollbackResp(*this); }
};

} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_PINGRESP_H */
