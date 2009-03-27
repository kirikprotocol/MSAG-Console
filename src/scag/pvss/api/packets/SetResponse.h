#ifndef _SCAG_PVSS_BASE_SETRESPONSE_H
#define _SCAG_PVSS_BASE_SETRESPONSE_H

#include "BatchResponseComponent.h"

namespace scag2 {
namespace pvss {

class SetResponse : public BatchResponseComponent
{
public:
    SetResponse() : BatchResponseComponent() {}
    SetResponse( uint32_t seqNum ) : BatchResponseComponent(seqNum) {}

    virtual bool visit( ResponseVisitor& visitor ) /* throw (PvapException) */  {
        return visitor.visitSetResponse(*this);
    }

    virtual SetResponse* clone() const { return new SetResponse(*this); }

protected:
    virtual const char* typeToString() const { return "set_resp"; }

private:
    SetResponse( const SetResponse& other ) : BatchResponseComponent(other) {}
    SetResponse& operator = ( const SetResponse& other );
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_SETRESPONSE_H */
