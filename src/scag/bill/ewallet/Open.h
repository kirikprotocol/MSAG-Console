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
    virtual ~Open() {}
    virtual std::string toString() const { return "open"; }
    virtual const char* typeToString() const { return "open"; }
    virtual bool isValid() const {
        return ( agentId_ != 0 ) && !userId_.empty() && (amount_!=0);
    }

    int32_t getAgentId() const { return agentId_; }
    const std::string& getUserId() const { return userId_; }
    const std::string& getWalletType() const { return walletType_; }
    const std::string& getDescription() const { return description_; }
    int32_t getAmount() const { return amount_; }
    int32_t getTimeout() const { return timeout_; }

    void setAgentId( int32_t s ) { agentId_ = s; }
    void setUserId( const std::string& uid ) { userId_ = uid; }
    void setWalletType( const std::string& wt ) { walletType_ = wt; }
    void setDescription( const std::string& d ) { description_ = d; }
    void setAmount( int32_t val ) { amount_ = val; }
    void setTimeout( int32_t tmo ) { timeout_ = tmo; }

    virtual bool visit( RequestVisitor& visitor ) {
        return visitor.visitOpen(*this);
    }

private:
    int32_t     agentId_;
    std::string userId_;
    std::string walletType_;
    std::string description_;
    int32_t     amount_;
    int32_t     timeout_;
};

} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_REQUEST_H */
