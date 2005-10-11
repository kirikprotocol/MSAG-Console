#ident "$Id$"
#ifndef __SMSC_INMAN_COMP_COMFACTORY__
#define __SMSC_INMAN_COMP_COMFACTORY__

#include <map>

#include "inman/comp/comps.hpp"
#include "inman/comp/acdefs.hpp"
#include "inman/comp/operfactory.hpp"
#include "logger/Logger.h"


namespace smsc {
namespace inman {
namespace comp {

typedef smsc::ac::CAP3SMSFactory  ComponentFactory;

OperationFactory * initCAP3SMSComponents(OperationFactory * fact);


}
}
}

#endif
