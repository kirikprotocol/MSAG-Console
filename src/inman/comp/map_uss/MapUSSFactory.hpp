#pragma ident "$Id$"
/* ************************************************************************* *
 * ROS Components factory for MAP Unstructured Supplementary Services (v2)
 * ************************************************************************* */
#ifndef __SMSC_INMAN_MAPUSS_COMFACTORY__
#define __SMSC_INMAN_MAPUSS_COMFACTORY__

#include "inman/comp/ROSFactory.hpp"
using smsc::inman::comp::ROSComponentsFactory;

namespace smsc {
namespace inman {
namespace comp {

extern EncodedOID _ac_map_networkUnstructuredSs_v2;

namespace uss {
//ROSFactoryProducer declaration
ROSComponentsFactory * initMAPUSS2Components(void);
} //uss

} //comp
} //inman
} //smsc

#endif /* __SMSC_INMAN_MAPUSS_COMFACTORY__ */

