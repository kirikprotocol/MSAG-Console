#ifndef _SCAG_PVSS_PVAP_TIMEPOLICYFILLER_H
#define _SCAG_PVSS_PVAP_TIMEPOLICYFILLER_H

#include <cassert>
#include "scag/pvss/packets/HavingProperty.h"
#include "BufferWriter.h"
#include "BufferReader.h"

namespace scag2 {
namespace pvss {
namespace pvap {

class TimePolicyFiller
{
public:
    TimePolicyFiller( HavingProperty& theOwner ) : owner(theOwner) {}
    TimePolicyFiller( HavingProperty* theOwner ) : owner(*theOwner) {
        assert(theOwner);
    }

    template < class Proto >
        void serialize( const Proto&, BufferWriter& writer ) const throw (PvapException)
    {
        // we don't check as it was checked before
        Property* p = owner.getProperty();
        writer.writeByte( uint8_t(p->getTimePolicy()) );
        writer.writeInt( p->getFinalDate() );
        writer.writeInt( p->getLifeTime() );
    }

    template < class Proto >
        void deserialize( Proto&, BufferReader& reader ) throw (PvapException)
    {
        try {
            uint8_t policyValue = reader.readByte();
            if ( policyValue == 0 ) {
                throw PvapSerializationException( owner.isRequest(),
                                                  owner.getSeqNum(),
                                                  "unknown time policy=%d",
                                                  ((int)policyValue & 0xff) );
            }
            int fd = int(reader.readInt());
            int lt = int(reader.readInt());
            if ( owner.getProperty() == 0 ) owner.setProperty( new Property() );
            owner.getProperty()->setTimePolicy( TimePolicy(policyValue), fd, lt );
        } catch ( exceptions::IOException& e ) {
            throw PvapSerializationException( owner.isRequest(),
                                              owner.getSeqNum(),
                                              "decoding time policy: %s",
                                              e.what() );
        }
    }

private:
    HavingProperty& owner;
};

} // namespace pvap
} // namespace pvss
} // namespace scag

#endif
