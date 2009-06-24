#include <cassert>
#include "ExternalBillingTransaction.h"

namespace scag2 {
namespace sessions {

using namespace util::io;

Serializer& ExternalBillingTransaction::serialize( Serializer& s ) const
{    
    s << id;
    s << uint64_t(billid_);
    s << (keywords_ ? *keywords_ : std::string());
    return s;
}


Deserializer& ExternalBillingTransaction::deserialize( Deserializer& s ) /* throw (DeserializerException) */
{
    uint32_t i;
    s >> i;
    assert( i == id );
    uint64_t ib;
    s >> ib;
    billid_ = ib;
    std::string kw;
    s >> kw;
    setKeywords( kw );
    return s;
}

}
}
