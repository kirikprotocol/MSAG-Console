#pragma ident "$Id$"
/* ************************************************************************* *
 * ROS Components factory for MAP AnyTimeInformationHandling service (v3)
 * ************************************************************************* */
#ifndef __SMSC_INMAN_MAPATIH_COMFACTORY__
#define __SMSC_INMAN_MAPATIH_COMFACTORY__

#include "inman/comp/ROSFactory.hpp"
using smsc::inman::comp::ROSComponentsFactory;

namespace smsc {
namespace inman {
namespace comp {

extern EncodedOID _ac_map_anyTimeInfoHandling_v3;

namespace atih {
//ROSFactoryProducer declaration
ROSComponentsFactory * initMAPATIH3Components(void);

} //atih
} //comp
} //inman
} //smsc

#endif /* __SMSC_INMAN_MAPATIH_COMFACTORY__ */

