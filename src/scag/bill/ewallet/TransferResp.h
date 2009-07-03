#ifndef SCAG_BILL_EWALLET_TRANSFERRESP_H
#define SCAG_BILL_EWALLET_TRANSFERRESP_H

#include "Response.h"
#include "ResponseVisitor.h"

namespace scag2 {
namespace bill {
namespace ewallet {

class TransferResp: public Response
{
public:
    TransferResp( uint8_t status = Status::UNKNOWN ) : Response(status) {}

    virtual std::string toString() const {
        std::string res;
        res.reserve(100);
        res.append(Response::toString());
        if ( getStatus() == Status::OK ) {
            char buf[100];
            if ( amount_ != 0 ) {
                snprintf(buf,sizeof(buf)," amount=%d", amount_);
                res.append(buf);
            }
            if ( chargeThreshold_ != 0 ) {
                snprintf(buf,sizeof(buf)," chargeThreshold=%d", chargeThreshold_);
                res.append(buf);
            }
        }
        return res;
    }
    virtual const char* typeToString() const { return "transferresp"; }
    virtual bool isValid() const { return true; }
    virtual bool visit( ResponseVisitor& visitor ) { return visitor.visitTransferResp(*this); }

    int32_t getAmount() const { return amount_; }
    int32_t getChargeThreshold() const { return amount_; }
    void setAmount( int32_t v ) { amount_ = v; }
    void setChargeThreshold( int32_t v ) { chargeThreshold_ = v; }

private:
    int32_t amount_;
    int32_t chargeThreshold_;
};

} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_CHECKRESP_H */
