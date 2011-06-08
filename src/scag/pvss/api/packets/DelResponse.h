#ifndef _SCAG_PVSS_BASE_DELRESPONSE_H
#define _SCAG_PVSS_BASE_DELRESPONSE_H

#include "BatchResponseComponent.h"

namespace scag2 {
namespace pvss {

class DelResponse : public BatchResponseComponent
{
public:
    DelResponse() : BatchResponseComponent() { initLog(); }
    DelResponse( uint8_t status ) : BatchResponseComponent(status) { initLog(); }
    virtual ~DelResponse() { logDtor(); }

    virtual bool visit( ProfileResponseVisitor& visitor ) /* throw (PvapException) */  {
        CHECKMAGTC;
        return visitor.visitDelResponse(*this);
    }

    virtual DelResponse* clone() const {
        CHECKMAGTC;
        return new DelResponse(*this);
    }

protected:
    virtual const char* typeToString() const { return "del_resp"; }

private:
    DelResponse( const DelResponse& other ) : BatchResponseComponent(other) {
        initLog();
    }
    DelResponse& operator = ( const DelResponse& other );
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_DELRESPONSE_H */
