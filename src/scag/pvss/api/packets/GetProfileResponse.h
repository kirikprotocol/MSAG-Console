#ifndef _SCAG_PVSS_BASE_GETPROFILERESPONSE_H
#define _SCAG_PVSS_BASE_GETPROFILERESPONSE_H

#include <vector>
#include "CommandResponse.h"
#include "GetProfileResponseComponent.h"

namespace scag2 {
namespace pvss {

class GetProfileResponse : public CommandResponse
{
public:
    GetProfileResponse() : CommandResponse() { initLog(); }
    GetProfileResponse( uint8_t status ) : CommandResponse(status) { initLog(); }
    virtual ~GetProfileResponse() { logDtor(); clear(); }

    virtual bool isValid( PvssException* exc = 0 ) const {
        return CommandResponse::isValid(exc);
    }

    virtual bool visit( ProfileResponseVisitor& visitor ) /* throw (PvapException) */  {
        return visitor.visitGetProfileResponse(*this);
    }

    virtual GetProfileResponse* clone() const { return new GetProfileResponse(*this); }

    void addComponent( GetProfileResponseComponent* resp ) {
        // if ( batchContent_.empty() || getStatus() == OK ) {
        // setStatus( resp->getStatus() );
        // }
        content_.push_back( resp );
    }

    const std::vector< GetProfileResponseComponent* >& getContent() const { return content_; }

    std::string toString() const {
        std::string rv(CommandResponse::toString());
        rv.reserve(200);
        rv += " [";
        bool comma = false;
        for ( std::vector< GetProfileResponseComponent* >::const_iterator i = content_.begin();
              i != content_.end();
              ++i ) {
            if ( comma ) rv += ", ";
            else comma = true;
            rv += (*i)->toString();
        }
        rv += "]";
        return rv;
    }

    void clear() {
        CommandResponse::clear();
        for ( std::vector< GetProfileResponseComponent* >::iterator i = content_.begin();
              i != content_.end();
              ++i ) {
            delete *i;
        }
        content_.clear();
    }

protected:
    virtual const char* typeToString() const { return "get_prof_resp"; }

private:
    GetProfileResponse( const GetProfileResponse& other ) : CommandResponse(other) {
        for ( std::vector< GetProfileResponseComponent* >::const_iterator i = other.content_.begin();
              i != other.content_.end();
              ++i ) {
            content_.push_back( (*i)->clone() );
        }
    }
    GetProfileResponse& operator = ( const GetProfileResponse& );

private:
    std::vector< GetProfileResponseComponent* > content_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_SETRESPONSE_H */
