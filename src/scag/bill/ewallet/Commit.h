#ifndef SCAG_BILL_EWALLET_COMMIT_H
#define SCAG_BILL_EWALLET_COMMIT_H

#include "Request.h"
#include "RequestVisitor.h"

namespace scag2 {
namespace bill {
namespace ewallet {

class Commit: public Request
{
public:
    Commit() : agentId_(0), amount_(0), transId_(0) {}

    virtual ~Commit() {
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
        snprintf(buf,sizeof(buf)," amount=%d", amount_ );
        res.append(buf);
        if (!externalId_.empty()) {
            snprintf(buf,sizeof(buf)," externalId=\"%s\"", externalId_.c_str() );
            res.append(buf);
        }
        snprintf(buf,sizeof(buf)," transId=%u", transId_ );
        res.append(buf);
        return res;
    }
    virtual const char* typeToString() const { return "commit"; }
    virtual bool isValid() const {
        CHECKMAGTC;
        return ( agentId_ != 0 ) && !userId_.empty();
    }

    const std::string& getSourceId() const { return sourceId_; }
    int32_t getAgentId() const { return agentId_; }
    const std::string& getUserId() const { return userId_; }
    const std::string& getWalletType() const { return walletType_; }
    int32_t getAmount() const { return amount_; }
    const std::string& getExternalId() const { return externalId_; }
    int32_t getTransId() const { return transId_; }

    void setSourceId( const std::string& si ) { sourceId_ = si; }
    void setAgentId( int32_t s ) { agentId_ = s; }
    void setUserId( const std::string& uid ) { userId_ = uid; }
    void setWalletType( const std::string& wt ) { walletType_ = wt; }
    void setAmount( int32_t val ) { amount_ = val; }
    void setExternalId( const std::string& ei ) { externalId_ = ei; }
    void setTransId( int32_t ti ) { transId_ = ti; }

    virtual bool visit( RequestVisitor& visitor ) {
        CHECKMAGTC;
        return visitor.visitCommit(*this);
    }

private:
    std::string sourceId_;
    int32_t     agentId_;
    std::string userId_;
    std::string walletType_;
    int32_t     amount_;
    std::string externalId_;
    int32_t     transId_;
};

} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_COMMIT_H */
