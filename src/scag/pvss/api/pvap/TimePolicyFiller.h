#ifndef _SCAG_PVSS_PVAP_TIMEPOLICYFILLER_H
#define _SCAG_PVSS_PVAP_TIMEPOLICYFILLER_H

#include <cassert>
#include "scag/pvss/api/packets/HavingProperty.h"
#include "BufferWriter.h"
#include "BufferReader.h"

namespace scag2 {
namespace pvss {
namespace pvap {

class TimePolicyFiller
{
public:
    TimePolicyFiller( HavingProperty* theOwner ) : owner_(theOwner) {
        assert(theOwner);
    }

    template < class Proto >
        void serialize( const Proto&, BufferWriter& writer ) const throw (PvapException)
    {
        // we don't check as it was checked before
        const Property& p = owner_->getProperty();
        writer.writeByte( uint8_t(p.getTimePolicy()) );
        writer.writeInt( int(p.getFinalDate()) );
        writer.writeInt( int(p.getLifeTime()) );
    }

    template < class Proto >
        void deserialize( Proto&, BufferReader& reader ) throw (PvapException)
    {
        try {
            uint8_t policyValue = reader.readByte();
            if ( policyValue == 0 ) {
                throw PvapSerializationException( owner_->isRequest(),
                                                  uint32_t(-1),
                                                  "unknown time policy=%d",
                                                  ((int)policyValue & 0xff) );
            }
            int fd = int(reader.readInt());
            int lt = int(reader.readInt());
            // if ( owner_->getProperty() == 0 ) owner_->setProperty( new Property() );
            owner_->getProperty().setTimePolicy( TimePolicy(policyValue), fd, lt );
        } catch ( exceptions::IOException& e ) {
            throw PvapSerializationException( owner_->isRequest(),
                                              uint32_t(-1),
                                              "decoding time policy: %s",
                                              e.what() );
        }
    }

private:
    HavingProperty* owner_;
};

} // namespace pvap
} // namespace pvss
} // namespace scag

#endif
