#ifndef SCAG_BILL_EWALLET_ROLLBACK_H
#define SCAG_BILL_EWALLET_ROLLBACK_H

#include "Request.h"
#include "RequestVisitor.h"

namespace scag2 {
namespace bill {
namespace ewallet {

class Rollback: public Request
{
public:
    virtual ~Rollback() {}
    virtual std::string toString() const { return "rollback"; }
    virtual const char* typeToString() const { return "rollback"; }
    virtual bool isValid() const {
        return ( agentId_ != 0 ) && !userId_.empty();
    }

    const std::string& getSourceId() const { return sourceId_; }
    int32_t getAgentId() const { return agentId_; }
    const std::string& getUserId() const { return userId_; }
    const std::string& getWalletType() const { return walletType_; }
    const std::string& getExternalId() const { return externalId_; }
    int32_t getTransId() const { return transId_; }

    void setSourceId( const std::string& si ) { sourceId_ = si; }
    void setAgentId( int32_t s ) { agentId_ = s; }
    void setUserId( const std::string& uid ) { userId_ = uid; }
    void setWalletType( const std::string& wt ) { walletType_ = wt; }
    void setExternalId( const std::string& ei ) { externalId_ = ei; }
    void setTransId( int32_t ti ) { transId_ = ti; }

    virtual bool visit( RequestVisitor& visitor ) {
        return visitor.visitRollback(*this);
    }

private:
    std::string sourceId_;
    int32_t     agentId_;
    std::string userId_;
    std::string walletType_;
    std::string externalId_;
    int32_t     transId_;
};

} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_ROLLBACK_H */
