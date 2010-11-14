/* ************************************************************************* *
 * ROS Components factory for MAP Unstructured Supplementary Services (v2)
 * ************************************************************************* */
#ifndef __SMSC_INMAN_MAPUSS_COMFACTORY__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_MAPUSS_COMFACTORY__

#include "inman/comp/ROSFactory.hpp"

namespace smsc {
namespace inman {
namespace comp {

using smsc::inman::comp::ROSComponentsFactory;

extern EncodedOID _ac_map_networkUnstructuredSs_v2;

namespace uss {
//ROSFactoryProducer declaration
ROSComponentsFactory * initMAPUSS2Components(void);
} //uss

} //comp
} //inman
} //smsc

#endif /* __SMSC_INMAN_MAPUSS_COMFACTORY__ */

