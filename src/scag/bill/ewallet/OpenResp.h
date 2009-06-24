#ifndef SCAG_BILL_EWALLET_OPENRESP_H
#define SCAG_BILL_EWALLET_OPENRESP_H

#include "Response.h"
#include "ResponseVisitor.h"

namespace scag2 {
namespace bill {
namespace ewallet {

class OpenResp: public Response
{
public:
    OpenResp( uint8_t status = Status::UNKNOWN ) : Response(status) {}
    virtual ~OpenResp() {}
    virtual std::string toString() const {
        char buf[100];
        snprintf(buf,sizeof(buf),"%s transId=%u amount=%d chargeThreshold=%d",
                 Response::toString().c_str(),
                 transId_, amount_, threshold_ );
        return buf;
    }
    virtual const char* typeToString() const { return "openresp"; }
    virtual bool isValid() const { return true; }
    virtual bool visit( ResponseVisitor& visitor ) { return visitor.visitOpenResp(*this); }

    int32_t getTransId() const { return transId_; }
    int32_t getAmount() const { return amount_; }
    int32_t getChargeThreshold() const { return threshold_; }

    void setTransId( int32_t ti ) { transId_ = ti; }
    void setAmount( int32_t am ) { amount_ = am; }
    void setChargeThreshold( int32_t ct ) { threshold_ = ct; }

private:
    int32_t  transId_;
    int32_t  amount_;
    int32_t  threshold_;
};

} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_PINGRESP_H */
