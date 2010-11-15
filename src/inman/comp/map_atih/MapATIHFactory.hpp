/* ************************************************************************* *
 * ROS Components factory for MAP AnyTimeInformationHandling service (v3)
 * ************************************************************************* */
#ifndef __SMSC_INMAN_MAPATIH_COMFACTORY__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_MAPATIH_COMFACTORY__

#include "inman/comp/ROSFactory.hpp"

namespace smsc {
namespace inman {
namespace comp {

using smsc::inman::comp::ROSComponentsFactory;

extern EncodedOID _ac_map_anyTimeInfoHandling_v3;

namespace atih {
//ROSFactoryProducer declaration
ROSComponentsFactory * initMAPATIH3Components(void);

} //atih
} //comp
} //inman
} //smsc

#endif /* __SMSC_INMAN_MAPATIH_COMFACTORY__ */

