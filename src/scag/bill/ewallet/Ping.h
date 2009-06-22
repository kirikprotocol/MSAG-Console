#ifndef SCAG_BILL_EWALLET_PING_H
#define SCAG_BILL_EWALLET_PING_H

#include "Request.h"
#include "RequestVisitor.h"

namespace scag2 {
namespace bill {
namespace ewallet {

class Ping: public Request
{
public:
    virtual ~Ping() {}
    virtual std::string toString() const { return "ping"; }
    virtual const char* typeToString() const { return "ping"; }
    virtual bool isValid() const { return true; }
    virtual bool visit( RequestVisitor& visitor ) { return visitor.visitPing(*this); }
};

} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_PING_H */
