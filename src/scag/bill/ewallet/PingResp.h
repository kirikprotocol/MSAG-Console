#ifndef SCAG_BILL_EWALLET_PINGRESP_H
#define SCAG_BILL_EWALLET_PINGRESP_H

#include "Response.h"

namespace scag2 {
namespace bill {
namespace ewallet {

class PingResp: public Response
{
public:
    virtual ~PingResp() {}
    virtual std::string toString() const { return "pingresp"; }
    virtual const char* typeToString() const { return "pingresp"; }
};

} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_PINGRESP_H */
