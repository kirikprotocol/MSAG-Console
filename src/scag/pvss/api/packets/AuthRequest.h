#ifndef _SCAG_PVSS_BASE_AUTHREQUEST_H
#define _SCAG_PVSS_BASE_AUTHREQUEST_H

#include "Request.h"
#include "RequestVisitor.h"

namespace scag2 {
namespace pvss {

///
/// ping request
///
class AuthRequest : public Request
{
public:
    AuthRequest() : protocolVersion_(0) { initLog(); }
    AuthRequest( uint32_t seqNum ) : Request(seqNum), protocolVersion_(0) { initLog(); }
    
public:
    virtual ~AuthRequest() { logDtor(); }

    virtual bool isValid( PvssException* exc = 0 ) const {
        do {
            if ( login_.empty() ) {
                if ( exc ) { *exc = PvssException("login is empty",PvssException::BAD_REQUEST); }
                break;
            }
            if ( password_.empty() ) {
                if ( exc ) { *exc = PvssException("password is empty",PvssException::BAD_REQUEST); }
                break;
            }
            if ( name_.empty() ) {
                if ( exc ) { *exc = PvssException("name is empty",PvssException::BAD_REQUEST); }
                break;
            }
            if ( protocolVersion_ == 0 ) {
                if ( exc ) { *exc = PvssException("protocol version is not set",PvssException::BAD_REQUEST); }
                break;
            }
            return true;
        } while ( false );
        return false;
    }
    /*
    virtual AuthRequest* getCommand() { return this; }
    virtual const AuthRequest* getCommand() const { return this; }
     */

    uint8_t getProtocolVersion() const { return protocolVersion_; }
    void setProtocolVersion( uint8_t a ) { protocolVersion_ = a; }

    const std::string& getLogin() const { return login_; }
    void setLogin( const std::string& login ) { login_ = login; }

    const std::string& getPassword() const { return password_; }
    void setPassword( const std::string& password ) { password_ = password; }

    const std::string& getName() const { return name_; }
    void setName( const std::string& name ) { name_ = name; }

    virtual std::string toString() const;
    virtual void clear() {
        login_.clear();
        password_.clear();
        name_.clear();
        protocolVersion_ = 0;
    }

    virtual bool visit( RequestVisitor& visitor ) /* throw (PvapException) */  {
        return visitor.visitAuthRequest(*this);
    }

    virtual AuthRequest* clone() const { return new AuthRequest(*this); }

protected:
    virtual const char* typeToString() const { return "auth"; }
    // virtual ResponseTypeMatch& getResponseTypeMatch() const;

private:
    AuthRequest( const AuthRequest& other ) :
    Request(other),
    protocolVersion_(other.protocolVersion_),
    login_(other.login_),
    password_(other.password_),
    name_(other.name_) {}

private:
    uint8_t     protocolVersion_;
    std::string login_;
    std::string password_;
    std::string name_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_AUTHREQUEST_H */
