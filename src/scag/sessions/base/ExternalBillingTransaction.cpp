#include <cassert>
#include "ExternalBillingTransaction.h"

namespace scag2 {
namespace sessions {

using namespace util::storage;

Serializer& ExternalBillingTransaction::serialize( Serializer& s ) const
{    
    s << id;
    s << uint32_t(billid_);
    return s;
}


Deserializer& ExternalBillingTransaction::deserialize( Deserializer& s ) throw (DeserializerException)
{
    uint32_t i;
    s >> i;
    assert( i == id );
    s >> i;
    billid_ = int(i);
    return s;
}

}
}
