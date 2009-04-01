/* ************************************************************************* *
 * Application Context registry - facility of ROS operations factories.
 * ************************************************************************* */
/* NOTE: ApplicationContextRegistry initialization is not a threadsafe !!!   */
#ifndef __ROS_AC_REGISTRY_HPP
#ident "@(#)$Id$"
#define __ROS_AC_REGISTRY_HPP

#include "core/synchronization/Mutex.hpp"
#include "eyeline/util/PObjRegistryT.hpp"
#include "eyeline/ros/ROSCompFactory.hpp"

namespace eyeline {
namespace ros {

using smsc::core::synchronization::Mutex;

class AppContextRegistry : 
          eyeline::util::POBJRegistry_T<EncodedOID, ROSComponentsFactory> {
protected:
    mutable Mutex   _sync;

    //forbid destruction and instantiation
    void* operator new(size_t);
    AppContextRegistry()
    { }
    ~AppContextRegistry()
    { }

public:
    static AppContextRegistry & get(void);
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


} //ros
} //eyeline


#endif /* __ROS_AC_REGISTRY_HPP */

