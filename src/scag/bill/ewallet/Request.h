#ifndef SCAG_BILL_EWALLET_REQUEST_H
#define SCAG_BILL_EWALLET_REQUEST_H

#include "Packet.h"

namespace scag2 {
namespace bill {
namespace ewallet {

class RequestVisitor;

class Request: public Packet
{
public:
    static bool isPing( const Request& req );

public:
    virtual ~Request() {}
    virtual bool isRequest() const { return true; }
    virtual const char* typeToString() const = 0;
    virtual bool visit( RequestVisitor& visitor ) = 0;
};

} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_REQUEST_H */
