#pragma ident "$Id$"
/* ************************************************************************* *
 * Application Context registry interface and definitions.
 * ************************************************************************* */
#ifndef __INMAN_INAP_ACREGISTRY_DEFS_HPP
#define __INMAN_INAP_ACREGISTRY_DEFS_HPP

#include "inman/comp/ROSFactory.hpp"
using smsc::inman::comp::ROSFactoryProducer;
using smsc::inman::comp::ROSComponentsFactory;
using smsc::inman::comp::EncodedOID;

namespace smsc {
namespace inman {
namespace inap {

class ApplicationContextRegistryITF {
public:
    virtual bool regFactory(ROSFactoryProducer fif) = 0;
    virtual const ROSComponentsFactory * getFactory(const EncodedOID & ac_oid) const = 0;
};

} //inap
} //inman
} //smsc

#endif /* __INMAN_INAP_ACREGISTRY_DEFS_HPP */

