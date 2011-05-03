#include "ExternalBillingTransaction.h"

namespace scag2 {
namespace sessions {

using namespace util::io;

Serializer& ExternalBillingTransaction::serialize( Serializer& s ) const
{    
    s << id;
    s << uint64_t(billid_);
    // NOTE: we don't need keywords anymore, but we still output
    // empty string to preserve the format
    const std::string empty;
    s << empty;
    return s;
}


Deserializer& ExternalBillingTransaction::deserialize( Deserializer& s ) /* throw (DeserializerException) */
{
    uint32_t i;
    s >> i;
    if ( i != id ) {
        throw smsc::util::Exception("billtrans: ids are not equal: i=%u id=%u",i,id);
    }
    uint64_t ib;
    s >> ib;
    billid_ = ib;
    // the same as above
    std::string kw;
    s >> kw;
    // setKeywords( kw );
    return s;
}

}
}
