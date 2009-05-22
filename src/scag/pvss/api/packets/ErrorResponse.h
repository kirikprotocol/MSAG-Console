#ifndef _SCAG_PVSS_BASE_ERRORRESPONSE_H
#define _SCAG_PVSS_BASE_ERRORRESPONSE_H

#include "AbstractNonProfileResponse.h"
#include "ResponseVisitor.h"

namespace scag2 {
namespace pvss {

class ErrorResponse : public AbstractNonProfileResponse
{
public:
    ErrorResponse() : AbstractNonProfileResponse() { initLog(); }
    ErrorResponse( uint32_t seqNum, uint8_t status = UNKNOWN, const char* msg = 0) :
    AbstractNonProfileResponse(seqNum,status), msg_(msg?msg:"") { initLog(); }
    virtual ~ErrorResponse() { logDtor(); }

    virtual bool visit( ResponseVisitor& visitor ) /* throw (PvapException) */  {
        return visitor.visitErrResponse(*this);
    }

    virtual ErrorResponse* clone() const { return new ErrorResponse(*this); }

    void setStatusMessage( const std::string& msg ) { msg_ = msg; }
    const std::string& getStatusMessage() const { return msg_; }

protected:
    virtual const char* typeToString() const { return "err_resp"; }
    virtual std::string statusHolderToString() const {
        char buf[256];
        snprintf(buf,sizeof(buf),"status=%s msg=%s",
                 statusToString(getStatus()), msg_.c_str());
        return buf;
    }

private:
    ErrorResponse( const ErrorResponse& other ) : AbstractNonProfileResponse(other) {
        initLog();
    }
    ErrorResponse& operator = ( const ErrorResponse& other );

private:
    std::string msg_;

};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_ERRORRESPONSE_H */
