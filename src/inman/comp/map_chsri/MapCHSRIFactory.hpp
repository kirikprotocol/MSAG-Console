#pragma ident "$Id$"
/* ************************************************************************* *
 * ROS Components factory for MAP CallHandling service SEND_ROUTING_INFO (v3)
 * ************************************************************************* */
#ifndef __SMSC_INMAN_MAPCHSRI_COMFACTORY__
#define __SMSC_INMAN_MAPCHSRI_COMFACTORY__

#include "inman/comp/ROSFactory.hpp"
using smsc::inman::comp::ROSComponentsFactory;

namespace smsc {
namespace inman {
namespace comp {

extern EncodedOID _ac_map_locInfoRetrieval_v3;

namespace chsri {
//ROSFactoryProducer declaration
ROSComponentsFactory * initMAPCHSRI3Components(void);
} //chsri

} //comp
} //inman
} //smsc

#endif /* __SMSC_INMAN_MAPCHSRI_COMFACTORY__ */

