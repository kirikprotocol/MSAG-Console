#ifndef SCAG_BILL_EWALLET_TRANSFER_H
#define SCAG_BILL_EWALLET_TRANSFER_H

#include "Request.h"
#include "RequestVisitor.h"

namespace scag2 {
namespace bill {
namespace ewallet {

class Transfer: public Request
{
public:
    Transfer() : agentId_(0), amount_(0) {}

    virtual ~Transfer() {}
    virtual std::string toString() const {
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
        snprintf(buf,sizeof(buf)," srcWalletType=\"%s\"", srcWalletType_.c_str() );
        res.append(buf);
        snprintf(buf,sizeof(buf)," dstWalletType=\"%s\"", dstWalletType_.c_str() );
        res.append(buf);
        snprintf(buf,sizeof(buf)," amount=%d", amount_ );
        res.append(buf);
        if (!description_.empty()) {
            snprintf(buf,sizeof(buf)," description=\"%s\"", description_.c_str() );
            res.append(buf);
        }
        if (!externalId_.empty()) {
            snprintf(buf,sizeof(buf)," externalId=\"%s\"", externalId_.c_str() );
            res.append(buf);
        }
        return res;
    }
    virtual const char* typeToString() const { return "transfer"; }
    virtual bool isValid() const {
        return ( agentId_ != 0 ) && !userId_.empty() && (amount_ != 0);
    }

    const std::string& getSourceId() const { return sourceId_; }
    int32_t getAgentId() const { return agentId_; }
    const std::string& getUserId() const { return userId_; }
    const std::string& getSrcWalletType() const { return srcWalletType_; }
    const std::string& getDstWalletType() const { return dstWalletType_; }
    const std::string& getDescription() const { return description_; }
    const std::string& getExternalId() const { return externalId_; }
    int32_t getAmount() const { return amount_; }

    void setSourceId( const std::string& v ) { sourceId_ = v; }
    void setAgentId( int32_t s ) { agentId_ = s; }
    void setUserId( const std::string& uid ) { userId_ = uid; }
    void setSrcWalletType( const std::string& wt ) { srcWalletType_ = wt; }
    void setDstWalletType( const std::string& wt ) { dstWalletType_ = wt; }
    void setExternalId( const std::string& ei ) { externalId_ = ei; }
    void setDescription( const std::string& v ) { description_ = v; }
    void setAmount( int32_t val ) { amount_ = val; }

    virtual bool visit( RequestVisitor& visitor ) {
        return visitor.visitTransfer(*this);
    }

private:
    std::string sourceId_;
    int32_t     agentId_;
    std::string userId_;
    std::string srcWalletType_;
    std::string dstWalletType_;
    std::string description_;
    int32_t     amount_;
    std::string externalId_;
};

} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_TRANSFER_H */
