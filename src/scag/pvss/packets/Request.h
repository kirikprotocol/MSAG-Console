#ifndef _SCAG_PVSS_BASE_REQUEST_H
#define _SCAG_PVSS_BASE_REQUEST_H

#include "Packet.h"
#include "scag/pvss/common/PvapException.h"

namespace scag2 {
namespace pvss {

class Command;
class Response;
class RequestVisitor;
class ResponseTypeMatch;

///
/// Abstract class Request
///
class Request : public Packet
{
protected:
    Request();

public:
    virtual bool isRequest() const { return true; }
    virtual Command* getCommand() = 0;
    virtual const Command* getCommand() const = 0;

    virtual bool visit( RequestVisitor& visitor ) throw (PvapException) = 0;
    bool matchResponseType( const Response& resp ) const;

protected:
    virtual ResponseTypeMatch& getResponseTypeMatch() const;

private:
    Request( const Request& );
    Request& operator = ( const Request& );
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_REQUEST_H */
