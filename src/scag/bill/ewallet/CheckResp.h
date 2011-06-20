#ifndef SCAG_BILL_EWALLET_CHECKRESP_H
#define SCAG_BILL_EWALLET_CHECKRESP_H

#include "Response.h"
#include "ResponseVisitor.h"

namespace scag2 {
namespace bill {
namespace ewallet {

class CheckResp: public Response
{
public:
    CheckResp( uint8_t status = Status::UNKNOWN ) : Response(status),
    transStatus_(0), amount_(0), enddate_(0) {}
    virtual ~CheckResp() {
        CHECKMAGTC;
    }
    virtual std::string toString() const {
        CHECKMAGTC;
        std::string res;
        res.reserve(100);
        res.append(Response::toString());
        if ( getStatus() == Status::OK ) {
            char buf[100];
            snprintf(buf,sizeof(buf)," transStatus=%u", transStatus_);
            res.append(buf);
            if (!sourceId_.empty()) {
                snprintf(buf,sizeof(buf)," sourceId=%s", sourceId_.c_str());
                res.append(buf);
            }
            if ( amount_ != 0 ) {
                snprintf(buf,sizeof(buf)," amount=%d", amount_);
                res.append(buf);
            }
            if ( enddate_ != 0 ) {
                const time_t tmt(enddate_);
                struct tm stm;
                localtime_r(&tmt,&stm);
                snprintf(buf,sizeof(buf)," enddate=%04u-%02u-%02u+%02u:%02u:%02u",
                         stm.tm_year+1900,stm.tm_mon+1,stm.tm_mday,
                         stm.tm_hour,stm.tm_min,stm.tm_sec);
                res.append(buf);
            }
        }
        return res;
    }
    virtual const char* typeToString() const { return "checkresp"; }
    virtual bool isValid() const { return true; }
    virtual bool visit( ResponseVisitor& visitor ) {
        CHECKMAGTC;
        return visitor.visitCheckResp(*this); 
    }

    int32_t getTransStatus() const { return transStatus_; }
    const std::string& getSourceId() const { return sourceId_; }
    int32_t getAmount() const { return amount_; }
    int64_t getEnddate() const { return enddate_; }

    void setTransStatus( int32_t ts ) { transStatus_ = ts; }
    void setSourceId( const std::string& si ) { sourceId_ = si; }
    void setAmount( int32_t ts ) { amount_ = ts; }
    void setEnddate( int64_t ct ) { enddate_ = ct; }

private:
    int32_t     transStatus_;
    std::string sourceId_;
    int32_t     amount_;
    int64_t     enddate_;
};

} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_CHECKRESP_H */
