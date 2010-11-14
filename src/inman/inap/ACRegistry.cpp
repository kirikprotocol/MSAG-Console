#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/inap/ACRegistry.hpp"

namespace smsc {
namespace inman {
namespace inap {

/* ************************************************************************* *
 * class ApplicationContextRegistry implementation
 * ************************************************************************* */

ApplicationContextRegistry & ApplicationContextRegistry::get(void)
{
    static ApplicationContextRegistry	ACFact;
    return ACFact;
}
//Registers the Operation factory, takes ownership!!!
//Returns false if there is a factory with same AC OID already registered.
bool ApplicationContextRegistry::regFactory(ROSFactoryProducer fif)
{
    MutexGuard  grd(_sync);
    std::auto_ptr<ROSComponentsFactory> fact(fif());
    bool rval = insert(fact->acOID(), fact.get());
    if (rval)
        fact.release();
    return rval;
}
//returns the operation factory for given AC, the factory should be
//initialized by preceeding call to ApplicationContextRegistry::Init()
const ROSComponentsFactory * 
    ApplicationContextRegistry::getFactory(const EncodedOID & ac_oid) const
{
    MutexGuard  grd(_sync);
    return (const ROSComponentsFactory*)find(ac_oid);
}



} //inap
} //inman
} //smsc


