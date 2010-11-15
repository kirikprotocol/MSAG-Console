/* ************************************************************************* *
 * ROS Components factory for MAP CallHandling service SEND_ROUTING_INFO (v3)
 * ************************************************************************* */
#ifndef __SMSC_INMAN_MAPCHSRI_COMFACTORY__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_MAPCHSRI_COMFACTORY__

#include "inman/comp/ROSFactory.hpp"

namespace smsc {
namespace inman {
namespace comp {

using smsc::inman::comp::ROSComponentsFactory;

extern EncodedOID _ac_map_locInfoRetrieval_v3;

namespace chsri {
//ROSFactoryProducer declaration
ROSComponentsFactory * initMAPCHSRI3Components(void);
} //chsri

} //comp
} //inman
} //smsc

#endif /* __SMSC_INMAN_MAPCHSRI_COMFACTORY__ */

