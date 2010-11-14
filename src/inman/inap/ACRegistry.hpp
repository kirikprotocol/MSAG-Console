/* ************************************************************************* *
 * Application Context registry - facility of ROS operations factories.
 * ************************************************************************* */
/* NOTE: ApplicationContextRegistry initialization is not a threadsafe one!! */
#ifndef __INMAN_INAP_ACREGISTRY_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_INAP_ACREGISTRY_HPP

#include "core/synchronization/Mutex.hpp"

#include "inman/common/ObjRegistryT.hpp"
#include "inman/inap/ACRegDefs.hpp"

namespace smsc {
namespace inman {
namespace inap {

using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;
using smsc::util::POBJRegistry_T;


class ApplicationContextRegistry : public ApplicationContextRegistryITF,
                                POBJRegistry_T<EncodedOID, ROSComponentsFactory> {
protected:
    mutable Mutex   _sync;

    //forbid destruction and instantiation
    void* operator new(size_t);
    ApplicationContextRegistry()
    { }
    ~ApplicationContextRegistry()
    { }

public:
    static ApplicationContextRegistry & get(void);
    // -----------------------------------------------------
    // -- ApplicationContextRegistryITF interface methods
    // -----------------------------------------------------
    //Registers the Operation factory, takes ownership!!!
    //Returns false if there is a factory with same AC OID already registered.
    bool regFactory(ROSFactoryProducer fif);
    //returns the operation factory for given AC, the factory should be
    //initialized by preceeding call to ApplicationContextRegistry::Init()
    const ROSComponentsFactory * getFactory(const EncodedOID & ac_oid) const;
};


} //inap
} //inman
} //smsc

#endif /* __INMAN_INAP_ACREGISTRY_HPP */

