#include <sys/time.h>
#include "sms/sms.h"
#include "SessionKey.h"

namespace scag2 {
namespace sessions {

    Serializer& SessionKey::serialize( Serializer& s ) const
    {
        return s << msisdn_;
    }


    Deserializer& SessionKey::deserialize( Deserializer& s ) throw (DeserializerException)
    {
        return s >> msisdn_;
    }


    const SessionKey& SessionKey::operator = ( const smsc::sms::Address& a )
    {
        // FIXME: number conversion ? maybe a method Address.toMSISDN() ?
        if ( a.plan != 1 ) {
            throw std::runtime_error( "address should have ISDN plan" );
        }

        uint64_t res = 0;
        for ( unsigned i = 0; i < a.length; ++i ) {
            const char c = a.value[i];
            if ( i == 0 && c == '+' ) continue;
            if ( c >= '0' && c <= '9' ) {
                res = res*10 + (c - '0');
            } else {
                throw std::runtime_error( "address '" + std::string(a.value) + "' has non-digit characters" );
            }
        }
        msisdn_ = res;
        return *this;
    }

    SessionPrimaryKey::SessionPrimaryKey( const SessionKey& k )
    {
        ::gettimeofday(&borntime_,0);
        key_ = k.toString();
        char buf[128];
        sprintf( buf,"/%ld%d", borntime_.tv_sec, int(borntime_.tv_usec/1000) );
        key_.append( buf );
    }


} // namespace sessions
} // namespace scag
