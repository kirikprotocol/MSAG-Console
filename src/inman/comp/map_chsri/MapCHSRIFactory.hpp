#ident "$Id$"
#ifndef __SMSC_INMAN_MAPCHSRI_COMFACTORY__
#define __SMSC_INMAN_MAPCHSRI_COMFACTORY__

#include "inman/comp/acdefs.hpp"
#include "inman/comp/operfactory.hpp"

using smsc::ac::ACOID;
using smsc::inman::comp::OperationFactory;
using smsc::inman::comp::OperationFactoryInstanceT;
using smsc::inman::comp::ApplicationContextFactory;

namespace smsc {
namespace inman {
namespace comp {
namespace chsri {

typedef OperationFactoryInstanceT <ACOID::id_ac_map_locInfoRetrieval_v3> MAPCHSRI3Factory;
    //FactoryInitFunction declaration
OperationFactory * initMAPCHSRI3Components(OperationFactory * fact);

} //chsri
} //comp
} //inman
} //smsc

#endif /* __SMSC_INMAN_MAPCHSRI_COMFACTORY__ */

