#ident "$Id$"
#ifndef __SMSC_INMAN_COMP_COMFACTORY__
#define __SMSC_INMAN_COMP_COMFACTORY__

#include <map>

#include "comps.hpp"

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

protected:
	ComponentMap products;
};

}
}
}

#endif
