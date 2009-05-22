#ifndef _SCAG_PVSS_BASE_AUTHRESPONSE_H
#define _SCAG_PVSS_BASE_AUTHRESPONSE_H

#include "AbstractNonProfileResponse.h"

namespace scag2 {
namespace pvss {

class AuthResponse : public AbstractNonProfileResponse
{
public:
    AuthResponse() : AbstractNonProfileResponse(), clientType_(0), sid_(0) { initLog(); }
    AuthResponse( uint32_t seqNum, uint8_t status = UNKNOWN ) :
    AbstractNonProfileResponse(seqNum,status), clientType_(0), sid_(0) {
        initLog();
    }
    virtual ~AuthResponse() { logDtor(); }

    virtual bool visit( ResponseVisitor& visitor ) /* throw (PvapException) */  {
        return visitor.visitAuthResponse(*this);
    }

    virtual AuthResponse* clone() const { return new AuthResponse(*this); }

    uint8_t getClientType() const { return clientType_; }
    void setClientType( uint8_t ct ) { clientType_ = ct; }
    bool hasClientType() const { return clientType_ != 0; }

    uint8_t getSid() const { return sid_; }
    void setSid( uint8_t ct ) { sid_ = ct; }
    bool hasSid() const { return sid_ != 0; }

protected:
    virtual const char* typeToString() const { return "auth_resp"; }
    virtual std::string statusHolderToString() const
    {
        char buf[100];
        snprintf(buf,sizeof(buf),"status=%s clientType=%u sid=%u",
                 statusToString(getStatus()), clientType_, sid_ );
        return buf;
    }


private:
    AuthResponse( const AuthResponse& other ) :
    AbstractNonProfileResponse(other), clientType_(other.clientType_), sid_(other.sid_) {
        initLog();
    }
    AuthResponse& operator = ( const AuthResponse& other );

private:
    uint8_t clientType_;
    uint8_t sid_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_AUTHRESPONSE_H */
