#ifndef _SCAG_PVSS_BASE_DELRESPONSE_H
#define _SCAG_PVSS_BASE_DELRESPONSE_H

#include "BatchResponseComponent.h"

namespace scag2 {
namespace pvss {

class DelResponse : public BatchResponseComponent
{
public:
    DelResponse() : BatchResponseComponent() {}
    DelResponse( uint32_t seqNum ) : BatchResponseComponent(seqNum) {}

    virtual bool visit( ResponseVisitor& visitor ) /* throw (PvapException) */  {
        return visitor.visitDelResponse(*this);
    }

    virtual DelResponse* clone() const { return new DelResponse(*this); }

protected:
    virtual const char* typeToString() const { return "del_resp"; }

private:
    DelResponse( const DelResponse& other ) : BatchResponseComponent(other) {}
    DelResponse& operator = ( const DelResponse& other );
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_DELRESPONSE_H */
