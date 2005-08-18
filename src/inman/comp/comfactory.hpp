#ident "$Id$"
#ifndef __SMSC_INMAN_COMP_COMFACTORY__
#define __SMSC_INMAN_COMP_COMFACTORY__

#include <map>

#include "comps.hpp"

namespace smsc {
namespace inman {
namespace comp{

using std::map;

namespace opcode
{
	enum
	{
		InitialDPSMS 				  = 60,
		FurnishChargingInformationSMS = 61,
		ConnectSMS					  = 62,
		RequestReportSMSEvent		  = 63,
		EventReportSMS				  = 64,
		ContinueSMS					  = 65,
		ReleaseSMS					  = 66,
		ResetTimerSMS				  = 67
	};
}

struct ComponentProducer
{
	virtual Component* create() const;
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
