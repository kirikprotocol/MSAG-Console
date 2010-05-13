#ifndef _SCAG_PVSS_PVAP_GETPROFILERESPONSEFILLER_H
#define _SCAG_PVSS_PVAP_GETPROFILERESPONSEFILLER_H

#include <cassert>
#include "Exceptions.h"
#include "scag/pvss/api/packets/GetProfileResponse.h"

namespace scag2 {
namespace pvss {
namespace pvap {

class PVAPPROF;
class BufferWriter;
class BufferReader;

class GetProfileResponseFiller
{
public:
    GetProfileResponseFiller( GetProfileResponse* theOwner ) : owner_(theOwner) {
        assert(theOwner);
    }

    void serialize( const PVAPPROF&, BufferWriter& writer ) const; // throw (PvapException);

    void deserialize( PVAPPROF&, BufferReader& reader ); // throw (PvapException);

    inline void push( GetProfileResponseComponent* comp ) {
        owner_->addComponent( comp );
    }
    inline void fail() throw(PvapException)
    {
        throw PvapSerializationException( false,
                                          uint32_t(-1),
                                          "cannot decode request as get_prof_resp content" );
    }

private:
    GetProfileResponse* owner_;
};

} // namespace pvap
} // namespace pvss
} // namespace scag

#endif
