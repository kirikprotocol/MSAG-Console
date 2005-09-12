#ident "$Id$"
#ifndef __SMSC_INMAN_COMP_COMFACTORY__
#define __SMSC_INMAN_COMP_COMFACTORY__

#include <map>

#include "comps.hpp"

#include "logger/Logger.h"

using smsc::logger::Logger;

namespace smsc {
namespace inman {
namespace comp{

using std::map;

struct ComponentProducer
{
	virtual Component* create() const { return NULL; }
};

class ComponentFactory
{

	typedef map<unsigned, ComponentProducer> ComponentMap;

public:
	ComponentFactory();
	virtual ~ComponentFactory();

	Component* createComponent(unsigned opcode);

	static ComponentFactory* getInstance();

protected:
	ComponentMap products;
	Logger* 	 logger;
};

}
}
}

#endif
