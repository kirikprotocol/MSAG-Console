#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/ros/ACRegistry.hpp"

namespace eyeline {
namespace ros {

using smsc::core::synchronization::MutexGuard;

/* ************************************************************************* *
 * class ApplicationContextRegistry implementation
 * ************************************************************************* */

AppContextRegistry & AppContextRegistry::get(void)
{
    static AppContextRegistry	ACFact;
    return ACFact;
}
//Registers the Operation factory, takes ownership!!!
//Returns false if there is a factory with same AC OID already registered.
bool AppContextRegistry::regFactory(ROSCompFactoryProducingFunc use_fif)
{
    MutexGuard  grd(_sync);
    std::auto_ptr<ROSComponentsFactory> fact(use_fif());
    bool rval = insert(fact->acOID(), fact.get());
    if (rval)
        fact.release();
    return rval;
}
//returns the operation factory for given AC, the factory should be
//initialized by preceeding call to AppContextRegistry::Init()
const ROSComponentsFactory * 
    AppContextRegistry::getFactory(const asn1::EncodedOID & ac_oid) const
{
    MutexGuard  grd(_sync);
    return (const ROSComponentsFactory*)find(ac_oid);
}



} //ros
} //eyeline

