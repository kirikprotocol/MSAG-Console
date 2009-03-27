#ifndef _SCAG_PVSS_BASE_AUTHREQUEST_H
#define _SCAG_PVSS_BASE_AUTHREQUEST_H

#include "AbstractNonProfileRequest.h"
#include "RequestVisitor.h"

namespace scag2 {
namespace pvss {

///
/// ping request
///
class AuthRequest : public AbstractNonProfileRequest
{
public:
    AuthRequest() : protocolVersion_(0) {}
    AuthRequest( uint32_t seqNum ) : AbstractNonProfileRequest(seqNum), protocolVersion_(0) {}
    
public:
    virtual bool isValid() const {
        return !login_.empty() && !password_.empty() && !name_.empty() && protocolVersion_ != 0;
    }
    virtual AuthRequest* getCommand() { return this; }
    virtual const AuthRequest* getCommand() const { return this; }

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
    virtual ResponseTypeMatch& getResponseTypeMatch() const;

private:
    AuthRequest( const AuthRequest& other ) :
    AbstractNonProfileRequest(other),
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
