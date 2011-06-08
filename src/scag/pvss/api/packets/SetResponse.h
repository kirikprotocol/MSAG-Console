#ifndef _SCAG_PVSS_BASE_SETRESPONSE_H
#define _SCAG_PVSS_BASE_SETRESPONSE_H

#include "BatchResponseComponent.h"

namespace scag2 {
namespace pvss {

class SetResponse : public BatchResponseComponent
{
public:
    SetResponse() : BatchResponseComponent() { initLog(); }
    SetResponse( uint8_t status ) : BatchResponseComponent(status) { initLog(); }
    virtual ~SetResponse() { logDtor(); }

    virtual bool visit( ProfileResponseVisitor& visitor ) /* throw (PvapException) */  {
        CHECKMAGTC;
        return visitor.visitSetResponse(*this);
    }

    virtual SetResponse* clone() const {
        CHECKMAGTC;
        return new SetResponse(*this);
    }

protected:
    virtual const char* typeToString() const { return "set_resp"; }

private:
    SetResponse( const SetResponse& other ) : BatchResponseComponent(other) { initLog(); }
    SetResponse& operator = ( const SetResponse& other );
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_SETRESPONSE_H */
