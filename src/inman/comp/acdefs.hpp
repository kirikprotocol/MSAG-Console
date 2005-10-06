#ident "$Id$"
#ifndef __SMSC_INMAN_COMP_APPCTXDEFS__
#define __SMSC_INMAN_COMP_APPCTXDEFS__

#include "inman/comp/operfactory.hpp"

using smsc::inman::comp::OperationFactoryInstanceT;

namespace smsc {
namespace ac {

#define id_ac_cap4_sms_AC			0
#define id_ac_map_networkUnstructuredSs_v2	1

typedef OperationFactoryInstanceT <id_ac_cap4_sms_AC> 
    CAP4SMSFactory;
typedef OperationFactoryInstanceT <id_ac_map_networkUnstructuredSs_v2>
    MAPUSS2Factory;

} //ac
} //smsc

#endif /* __SMSC_INMAN_COMP_APPCTXDEFS__ */
