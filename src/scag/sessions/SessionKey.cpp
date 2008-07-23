#include "SessionKey.h"

namespace scag {
namespace sessions2 {

    Serializer& SessionKey::serialize( Serializer& s ) const
    {
        return s << msisdn_;
    }


    Deserializer& SessionKey::deserialize( Deserializer& s ) throw (DeserializerException)
    {
        return s >> msisdn_;
    }

} // namespace sessions
} // namespace scag
