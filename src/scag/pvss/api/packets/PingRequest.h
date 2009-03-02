#ifndef _SCAG_PVSS_BASE_PINGREQUEST_H
#define _SCAG_PVSS_BASE_PINGREQUEST_H

#include "AbstractNonProfileRequest.h"
#include "RequestVisitor.h"

namespace scag2 {
namespace pvss {

///
/// ping request
///
class PingRequest : public AbstractNonProfileRequest
{
public:
    PingRequest() {}
    PingRequest( uint32_t seqNum ) : AbstractNonProfileRequest(seqNum) {}
    
public:
    virtual bool isValid() const { return true; }
    virtual PingRequest* getCommand() { return this; }
    virtual const PingRequest* getCommand() const { return this; }
    virtual const char* typeToString() const { return "ping"; }
    virtual void clear() {}

    virtual bool visit( RequestVisitor& visitor ) throw (PvapException) {
        return visitor.visitPingRequest(*this);
    }

    virtual PingRequest* clone() const { return new PingRequest(*this); }

    virtual bool isPing() const { return true; }

protected:
    virtual ResponseTypeMatch& getResponseTypeMatch() const;

};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_PINGREQUEST_H */
