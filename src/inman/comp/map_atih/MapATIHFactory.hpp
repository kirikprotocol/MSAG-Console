#ident "$Id$"
#ifndef __SMSC_INMAN_MAPATIH_COMFACTORY__
#define __SMSC_INMAN_MAPATIH_COMFACTORY__

#include "inman/comp/acdefs.hpp"
#include "inman/comp/operfactory.hpp"

using smsc::ac::ACOID;
using smsc::inman::comp::OperationFactory;
using smsc::inman::comp::OperationFactoryInstanceT;
using smsc::inman::comp::ApplicationContextFactory;

namespace smsc {
namespace inman {
namespace comp {
namespace atih {

typedef OperationFactoryInstanceT <ACOID::id_ac_map_anyTimeInfoHandling_v3> MAPATIH3Factory;
//FactoryInitFunction declaration
OperationFactory * initMAPATIH3Components(OperationFactory * fact);

} //atih
} //comp
} //inman
} //smsc

#endif /* __SMSC_INMAN_MAPATIH_COMFACTORY__ */

