#ifndef SCAG_BILL_EWALLET_COMMITRESP_H
#define SCAG_BILL_EWALLET_COMMITRESP_H

#include "Response.h"
#include "ResponseVisitor.h"

namespace scag2 {
namespace bill {
namespace ewallet {

class CommitResp: public Response
{
public:
    CommitResp( uint8_t status = Status::UNKNOWN ) : Response(status) {}
    virtual ~CommitResp() {}
    virtual const char* typeToString() const { return "commitresp"; }
    virtual bool isValid() const { return true; }
    virtual bool visit( ResponseVisitor& visitor ) { return visitor.visitCommitResp(*this); }
};

} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_PINGRESP_H */
