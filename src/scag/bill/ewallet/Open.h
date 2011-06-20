#ifndef SCAG_BILL_EWALLET_OPEN_H
#define SCAG_BILL_EWALLET_OPEN_H

#include "Request.h"
#include "RequestVisitor.h"

namespace scag2 {
namespace bill {
namespace ewallet {

class Open: public Request
{
public:
    Open() : agentId_(0), amount_(0), timeout_(0) {}

    virtual ~Open() {
        CHECKMAGTC;
    }
    virtual std::string toString() const {
        CHECKMAGTC;
        std::string res;
        res.reserve(100);
        res.append( Request::toString() );
        char buf[100];
        snprintf(buf,sizeof(buf)," sourceId=\"%s\"",sourceId_.c_str());
        res.append(buf);
        snprintf(buf,sizeof(buf)," agentId=%u", agentId_ );
        res.append(buf);
        snprintf(buf,sizeof(buf)," userId=\"%s\"", userId_.c_str() );
        res.append(buf);
        snprintf(buf,sizeof(buf)," walletType=\"%s\"", walletType_.c_str() );
        res.append(buf);
        if (!description_.empty()) {
            snprintf(buf,sizeof(buf)," description=\"%s\"", description_.c_str() );
            res.append(buf);
        }
        snprintf(buf,sizeof(buf)," amount=%d", amount_ );
        res.append(buf);
        if (!externalId_.empty()) {
            snprintf(buf,sizeof(buf)," externalId=\"%s\"", externalId_.c_str() );
            res.append(buf);
        }
        snprintf(buf,sizeof(buf)," timeout=%d", timeout_ );
        res.append(buf);
        return res;
    }
    virtual const char* typeToString() const { return "open"; }
    virtual bool isValid() const {
        CHECKMAGTC;
        return ( agentId_ != 0 ) && !userId_.empty();
    }

    const std::string& getSourceId() const { return sourceId_; }
    int32_t getAgentId() const { return agentId_; }
    const std::string& getUserId() const { return userId_; }
    const std::string& getWalletType() const { return walletType_; }
    const std::string& getDescription() const { return description_; }
    int32_t getAmount() const { return amount_; }
    const std::string& getExternalId() const { return externalId_; }
    int32_t getTimeout() const { return timeout_; }

    void setSourceId( const std::string& si ) { sourceId_ = si; }
    void setAgentId( int32_t s ) { agentId_ = s; }
    void setUserId( const std::string& uid ) { userId_ = uid; }
    void setWalletType( const std::string& wt ) { walletType_ = wt; }
    void setDescription( const std::string& d ) { description_ = d; }
    void setAmount( int32_t val ) { amount_ = val; }
    void setExternalId( const std::string& ei ) { externalId_ = ei; }
    void setTimeout( int32_t tmo ) { timeout_ = tmo; }

    virtual bool visit( RequestVisitor& visitor ) {
        CHECKMAGTC;
        return visitor.visitOpen(*this);
    }

private:
    std::string sourceId_;
    int32_t     agentId_;
    std::string userId_;
    std::string walletType_;
    std::string description_;
    int32_t     amount_;
    std::string externalId_;
    int32_t     timeout_;
};

} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_REQUEST_H */
