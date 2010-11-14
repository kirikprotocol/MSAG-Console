/* ************************************************************************* *
 * ROS Components factory for CAMEL phase 3 SMS service.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_CAPSMS_COMFACTORY__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_CAPSMS_COMFACTORY__

#include "inman/comp/ROSFactory.hpp"
using smsc::inman::comp::ROSComponentsFactory;

namespace smsc {
namespace inman {
namespace comp {

extern EncodedOID _ac_cap3_sms;
//ROSFactoryProducer declaration
ROSComponentsFactory * initCAP3SMSComponents(void);

} //comp
} //inman
} //smsc

#endif /* __SMSC_INMAN_CAPSMS_COMFACTORY__ */

