#include <cassert>
#include "ExternalBillingTransaction.h"

namespace scag2 {
namespace sessions {

using namespace util::storage;

Serializer& ExternalBillingTransaction::serialize( Serializer& s ) const
{    
    s << id;
    s << uint32_t(billid_);
    s << getKeywords();
    return s;
}


Deserializer& ExternalBillingTransaction::deserialize( Deserializer& s ) throw (DeserializerException)
{
    uint32_t i;
    s >> i;
    assert( i == id );
    s >> i;
    billid_ = int(i);
    std::string kw;
    s >> kw;
    setKeywords( kw );
    return s;
}

}
}
