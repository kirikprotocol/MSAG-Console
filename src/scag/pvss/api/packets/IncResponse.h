#ifndef _SCAG_PVSS_BASE_INCRESPONSE_H
#define _SCAG_PVSS_BASE_INCRESPONSE_H

#include "BatchResponseComponent.h"

namespace scag2 {
namespace pvss {

class IncResponse : public BatchResponseComponent
{
public:
    IncResponse() : BatchResponseComponent(), result_(0), incResult_(true) { initLog(); }
    IncResponse( uint8_t status ) : BatchResponseComponent(status), result_(0), incResult_(true) { initLog(); }
    virtual ~IncResponse() { logDtor(); }

    virtual bool visit( ProfileResponseVisitor& visitor ) /* throw (PvapException) */  {
        CHECKMAGTC;
        return visitor.visitIncResponse(*this);
    }

    virtual IncResponse* clone() const {
        CHECKMAGTC;
        return new IncResponse(*this);
    }

    virtual std::string toString() const {
        CHECKMAGTC;
        char buf[48];
        snprintf(buf,sizeof(buf)," result=%u",unsigned(result_));
        return BatchResponseComponent::toString() + buf;
    }

    uint32_t getResult() const { return result_; }
    void setResult( uint32_t result ) { result_ = result; }

    bool isIncResult() const { return incResult_; }
    void setIncResult(bool incResult) { incResult_ = incResult; }

protected:
    virtual const char* typeToString() const { return "inc_resp"; }

private:
    IncResponse( const IncResponse& other ) : BatchResponseComponent(other), result_(other.result_) {
        initLog();
    }
    IncResponse& operator = ( const IncResponse& other );

private:
    uint32_t result_;
    bool incResult_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_SETRESPONSE_H */
