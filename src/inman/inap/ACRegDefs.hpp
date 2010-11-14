/* ************************************************************************* *
 * Application Context registry interface and definitions.
 * ************************************************************************* */
#ifndef __INMAN_INAP_ACREGISTRY_DEFS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_INAP_ACREGISTRY_DEFS_HPP

#include "inman/comp/ROSFactory.hpp"

namespace smsc {
namespace inman {
namespace inap {

using smsc::inman::comp::ROSFactoryProducer;
using smsc::inman::comp::ROSComponentsFactory;
using smsc::inman::comp::EncodedOID;

class ApplicationContextRegistryITF {
protected:
    virtual ~ApplicationContextRegistryITF() //forbid interface destruction
    { }

public:
    virtual bool regFactory(ROSFactoryProducer fif) = 0;
    virtual const ROSComponentsFactory * getFactory(const EncodedOID & ac_oid) const = 0;
};

} //inap
} //inman
} //smsc

#endif /* __INMAN_INAP_ACREGISTRY_DEFS_HPP */

