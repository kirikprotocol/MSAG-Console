#ifndef _SCAG_PVSS_BASE_PINGREQUEST_H
#define _SCAG_PVSS_BASE_PINGREQUEST_H

#include "Request.h"
#include "RequestVisitor.h"

namespace scag2 {
namespace pvss {

///
/// ping request
///
class PingRequest : public Request
{
public:
    PingRequest() { /*initLog();*/ }
    PingRequest( uint32_t seqNum ) : Request(seqNum) { /*initLog();*/ }
    virtual ~PingRequest() { /*logDtor();*/ }
    
public:
    virtual bool isValid( PvssException* = 0 ) const { return true; }
    // virtual PingRequest* getCommand() { return this; }
    // virtual const PingRequest* getCommand() const { return this; }
    virtual const char* typeToString() const { return "ping"; }
    virtual void clear() {}

    virtual bool visit( RequestVisitor& visitor ) /* throw (PvapException) */  {
        return visitor.visitPingRequest(*this);
    }

    virtual PingRequest* clone() const { return new PingRequest(*this); }

    virtual bool isPing() const { return true; }

protected:
    // virtual ResponseTypeMatch& getResponseTypeMatch() const;

};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_PINGREQUEST_H */
