#ifndef TEST_SMEMAN_SME_MANAGER_UTIL
#define TEST_SMEMAN_SME_MANAGER_UTIL

#include "smeman/smeproxy.h"
#include "smeman/smsccmd.h"

namespace smsc {
namespace test {
namespace smeman {

using namespace smsc::smeman; //SmeProxy, SmeProxyPriority, SmeProxyState, ...
using namespace smsc::test::util; //rand0()

//max values
const int MAX_SYSTEM_ID_LENGTH = 15;
const int MAX_PASSWORD_LENGTH = 8;
const int MAX_SYSTEM_TYPE_LENGTH = 12;
const int MAX_ADDRESS_RANGE_LENGTH = 40;

class CorrectSmeProxy : public SmeProxy
{
	SmeProxyPriority priority;
	bool input;
	void* monitor; 
public:
	CorrectSmeProxy()
		: priority(rand2(SmeProxyPriorityMin, SmeProxyPriorityMax)),
		input(rand0(1)), monitor(0) {}
	virtual void close() {}
	virtual void putCommand(const SmscCommand& command) {}
	virtual SmscCommand getCommand()
	{
		input = rand0(1);
		return SmscCommand();
	}
	virtual SmeProxyState getState() const { return VALID; }
	virtual void init() {}
	virtual SmeProxyPriority getPriority() const
	{
		return priority;
	}
	virtual bool hasInput() const { return input; }
	virtual void attachMonitor(ProxyMonitor *m) { monitor = m; }
	virtual bool attached() { return monitor; }
	virtual uint32_t getNextSequenceNumber() { return 0; }
	//virtual uint32_t getUniqueId() const { return 0; }
};

class IncorrectSmeProxy : public CorrectSmeProxy
{
public:
	IncorrectSmeProxy() {}
	virtual SmeProxyPriority getPriority() const
	{
		switch (rand0(1))
		{
			case 0:
				return SmeProxyPriorityMin - 1;
			case 1:
				return SmeProxyPriorityMax + 1;
		}
	}
};

}
}
}

#endif /* TEST_SMEMAN_SME_MANAGER_UTIL */

