#ident "$Id$"
#ifndef __SMSC_INMAN_MAPUSS_COMFACTORY__
#define __SMSC_INMAN_MAPUSS_COMFACTORY__

#include "inman/comp/acdefs.hpp"
#include "inman/comp/operfactory.hpp"

using smsc::ac::ACOID;
using smsc::inman::comp::OperationFactory;
using smsc::inman::comp::OperationFactoryInstanceT;
using smsc::inman::comp::ApplicationContextFactory;

namespace smsc {
namespace inman {
namespace comp {
namespace uss {

typedef OperationFactoryInstanceT <ACOID::id_ac_map_networkUnstructuredSs_v2> MAPUSS2Factory;
    //FactoryInitFunction declaration
OperationFactory * initMAPUSS2Components(OperationFactory * fact);

} //uss
} //comp
} //inman
} //smsc

#endif /* __SMSC_INMAN_MAPUSS_COMFACTORY__ */

