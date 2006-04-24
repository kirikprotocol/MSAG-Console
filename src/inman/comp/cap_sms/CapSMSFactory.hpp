#ident "$Id$"
#ifndef __SMSC_INMAN_CAPSMS_COMFACTORY__
#define __SMSC_INMAN_CAPSMS_COMFACTORY__

#include "inman/comp/acdefs.hpp"
#include "inman/comp/operfactory.hpp"

using smsc::ac::ACOID;
using smsc::inman::comp::OperationFactory;
using smsc::inman::comp::OperationFactoryInstanceT;
using smsc::inman::comp::ApplicationContextFactory;

namespace smsc {
namespace inman {
namespace comp {

typedef OperationFactoryInstanceT <ACOID::id_ac_cap3_sms_AC> CAP3SMSFactory;

OperationFactory * initCAP3SMSComponents(OperationFactory * fact);

} //comp
} //inman
} //smsc

#endif /* __SMSC_INMAN_CAPSMS_COMFACTORY__ */

