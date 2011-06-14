/* ************************************************************************* *
 * ROS Components factory for MAP Unstructured Supplementary Services (v2)
 * ************************************************************************* */
#ifndef __SMSC_USSMAN_MAPUSS_COMFACTORY__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_USSMAN_MAPUSS_COMFACTORY__

#include "inman/comp/ROSFactory.hpp"

namespace smsc {
namespace ussman {
namespace comp {

using smsc::inman::comp::ROSComponentsFactory;
using smsc::inman::comp::EncodedOID;

extern EncodedOID _ac_map_networkUnstructuredSs_v2;

namespace uss {
//ROSFactoryProducer declaration
ROSComponentsFactory * initMAPUSS2Components(void);
} //uss

} //comp
} //ussman
} //smsc

#endif /* __SMSC_USSMAN_MAPUSS_COMFACTORY__ */

