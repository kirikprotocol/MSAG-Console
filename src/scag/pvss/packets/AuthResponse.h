#ifndef _SCAG_PVSS_BASE_AUTHRESPONSE_H
#define _SCAG_PVSS_BASE_AUTHRESPONSE_H

#include "Response.h"

namespace scag2 {
namespace pvss {

class AuthResponse : public Response
{
public:
    AuthResponse() : Response(), clientType_(0), sid_(0) {}
    AuthResponse( uint32_t seqNum ) : Response(seqNum), clientType_(0), sid_(0) {}

    virtual bool visit( ResponseVisitor& visitor ) throw (PvapException) {
        return visitor.visitAuthResponse(*this);
    }

    uint8_t getClientType() const { return clientType_; }
    void setClientType( uint8_t ct ) { clientType_ = ct; }
    bool hasClientType() const { return clientType_ != 0; }

    uint8_t getSid() const { return sid_; }
    void setSid( uint8_t ct ) { sid_ = ct; }
    bool hasSid() const { return sid_ != 0; }

protected:
    virtual const char* typeToString() const { return "auth_resp"; }

private:
    AuthResponse( const AuthResponse& other );
    AuthResponse& operator = ( const AuthResponse& other );

private:
    uint8_t clientType_;
    uint8_t sid_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_AUTHRESPONSE_H */
