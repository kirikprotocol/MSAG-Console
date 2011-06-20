#ifndef SCAG_BILL_EWALLET_INFO_H
#define SCAG_BILL_EWALLET_INFO_H

#include "Request.h"
#include "RequestVisitor.h"

namespace scag2 {
namespace bill {
namespace ewallet {

class Info : public Request
{
public:
    Info() : agentId_(0) {}
    virtual ~Info() {
        CHECKMAGTC;
    }
    virtual std::string toString() const {
        CHECKMAGTC;
        std::string res;
        res.reserve(100);
        res.append( Request::toString() );
        char buf[100];
        snprintf(buf,sizeof(buf)," agentId=%u", agentId_ );
        res.append(buf);
        snprintf(buf,sizeof(buf)," userId=\"%s\"", userId_.c_str() );
        res.append(buf);
        snprintf(buf,sizeof(buf)," walletType=\"%s\"", walletType_.c_str() );
        res.append(buf);
        return res;
    }
    virtual const char* typeToString() const { return "info"; }
    virtual bool isValid() const {
        CHECKMAGTC;
        return ( agentId_ != 0 ) && !userId_.empty();
    }

    int32_t getAgentId() const { return agentId_; }
    const std::string& getUserId() const { return userId_; }
    const std::string& getWalletType() const { return walletType_; }

    void setAgentId( int32_t s ) { agentId_ = s; }
    void setUserId( const std::string& uid ) { userId_ = uid; }
    void setWalletType( const std::string& wt ) { walletType_ = wt; }

    virtual bool visit( RequestVisitor& visitor ) {
        CHECKMAGTC;
        return visitor.visitInfo(*this);
    }

private:
    int32_t     agentId_;
    std::string userId_;
    std::string walletType_;
};

}
}
}

#endif

