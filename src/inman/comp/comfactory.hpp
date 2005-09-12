#ident "$Id$"
#ifndef __SMSC_INMAN_COMP_COMFACTORY__
#define __SMSC_INMAN_COMP_COMFACTORY__

#include <map>

#include "comps.hpp"
#include "inman/common/factory.hpp"
#include "logger/Logger.h"

using smsc::logger::Logger;
using smsc::inman::common::FactoryT;

namespace smsc {
namespace inman {
namespace comp{

using std::map;

class ComponentFactory : public FactoryT< unsigned, Component >
{
public:
	ComponentFactory();
	virtual ~ComponentFactory();

	static ComponentFactory* getInstance();

protected:
	Logger* 	 logger;
};

}
}
}

#endif
