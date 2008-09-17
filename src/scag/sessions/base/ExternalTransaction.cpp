#include "ExternalTransaction.h"
#include "ExternalBillingTransaction.h"

namespace scag2 {
namespace sessions {

using namespace util::storage;

ExternalTransaction* ExternalTransaction::createAndDeserialize( Deserializer& s ) throw (DeserializerException)
{
    ExternalTransaction* e = 0;
    uint32_t i;
    {
        size_t p = s.rpos();
        s >> i;
        s.setrpos(p);
    }
    switch (int(i)) {
    case (ExternalBillingTransaction::id) :
        e = new ExternalBillingTransaction( 0 );
        break;
    default:
        break;
    };
    if ( e ) e->deserialize( s );
    return e;
}

}
}
