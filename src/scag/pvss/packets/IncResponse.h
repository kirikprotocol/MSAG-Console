#ifndef _SCAG_PVSS_BASE_INCRESPONSE_H
#define _SCAG_PVSS_BASE_INCRESPONSE_H

#include "BatchResponseComponent.h"

namespace scag2 {
namespace pvss {

class IncResponse : public BatchResponseComponent
{
public:
    IncResponse() : BatchResponseComponent(), result_(0) {}
    IncResponse( uint32_t seqNum ) : BatchResponseComponent(seqNum), result_(0) {}

    virtual bool visit( ResponseVisitor& visitor ) throw (PvapException) {
        return visitor.visitIncResponse(*this);
    }

    uint32_t getResult() const { return result_; }
    void setResult( uint32_t result ) { result_ = result; }

protected:
    virtual const char* typeToString() const { return "inc_resp"; }

private:
    IncResponse( const IncResponse& other );
    IncResponse& operator = ( const IncResponse& other );

private:
    uint32_t result_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_SETRESPONSE_H */
