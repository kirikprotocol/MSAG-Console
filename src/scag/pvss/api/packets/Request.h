#ifndef _SCAG_PVSS_BASE_REQUEST_H
#define _SCAG_PVSS_BASE_REQUEST_H

#include "Packet.h"
#include "scag/pvss/common/PvapException.h"
#include "util/TypeInfo.h"

namespace scag2 {
namespace pvss {

class Command;
class Response;
class RequestVisitor;
// class ResponseTypeMatch;

///
/// Abstract class Request
///
class Request : public Packet
{
protected:
    Request() : seqNum_(-1) {}
    Request( uint32_t seqNum ) : seqNum_(seqNum) {}

public:
    virtual ~Request() {}

    virtual uint32_t getSeqNum() const {
        return seqNum_;
    }
    virtual void setSeqNum( uint32_t seqNum ) {
        CHECKMAGTC;
        seqNum_ = seqNum;
    }
    virtual std::string toString() const {
        CHECKMAGTC;
        char buf[48];
        snprintf( buf, sizeof(buf), "seqNum=%d %s", seqNum_, typeToString() );
        return buf;
    }

    virtual bool isRequest() const { return true; }
    virtual bool isPing() const { return false; }

    // virtual Command* getCommand() = 0;
    // virtual const Command* getCommand() const = 0;

    virtual bool visit( RequestVisitor& visitor ) = 0;
    virtual Request* clone() const = 0;

    // bool matchResponseType( const Response& resp ) const;

    /// merge timing measurements from response
    virtual void mergeTiming( const Response& ) {}

protected:
    virtual const char* typeToString() const = 0;
    Request( const Request& other ) : Packet(other), seqNum_(other.seqNum_) {}

private:
    Request& operator = ( const Request& );

private:
    DECLMAGTC(Request);
    uint32_t seqNum_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_REQUEST_H */
