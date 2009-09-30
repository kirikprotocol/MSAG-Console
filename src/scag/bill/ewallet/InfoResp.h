#ifndef SCAG_BILL_EWALLET_INFORESP_H
#define SCAG_BILL_EWALLET_INFORESP_H

#include "Response.h"
#include "ResponseVisitor.h"

namespace scag2 {
namespace bill {
namespace ewallet {

class InfoResp: public Response
{
public:
    InfoResp( uint8_t status = Status::UNKNOWN ) : Response(status) {}
    virtual ~InfoResp() {}
    virtual std::string toString() const {
        if ( getStatus() != Status::OK ) {
            return Response::toString();
        } else {
            char buf[100];
            snprintf(buf,sizeof(buf),"%s amount=%d chargeThreshold=%d",
                     Response::toString().c_str(),
                     amount_, threshold_ );
            return buf;
        }
    }
    virtual const char* typeToString() const { return "inforesp"; }
    virtual bool isValid() const { return true; }
    virtual bool visit( ResponseVisitor& visitor ) { return visitor.visitInfoResp(*this); }

    int32_t getAmount() const { return amount_; }
    int32_t getChargeThreshold() const { return threshold_; }

    void setAmount( int32_t am ) { amount_ = am; }
    void setChargeThreshold( int32_t ct ) { threshold_ = ct; }

private:
    int32_t  amount_;
    int32_t  threshold_;
};

} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_PINGRESP_H */
