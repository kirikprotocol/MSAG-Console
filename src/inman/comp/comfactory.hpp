#ident "$Id$"
#ifndef __SMSC_INMAN_COMP_COMFACTORY__
#define __SMSC_INMAN_COMP_COMFACTORY__

#include <map>

#include "comps.hpp"
#include "acdefs.hpp"
#include "inman/comp/operfactory.hpp"
#include "logger/Logger.h"

//using smsc::logger::Logger;
//using smsc::inman::common::FactoryT;

namespace smsc {
namespace inman {
namespace comp {

typedef smsc::ac::CAP4SMSFactory  ComponentFactory;

void initCAP4SMSComponents(OperationFactory * fact);


}
}
}

#endif
