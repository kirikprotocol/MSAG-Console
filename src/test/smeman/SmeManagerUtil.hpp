#ifndef TEST_SMEMAN_SME_MANAGER_UTIL
#define TEST_SMEMAN_SME_MANAGER_UTIL

#include "smeman/smeproxy.h"
#include "smeman/smsccmd.h"

namespace smsc {
namespace test {
namespace smeman {

using namespace smsc::smeman; //SmeProxy, SmeProxyPriority, SmeProxyState, ...

//max values
const int MAX_SYSTEM_ID_LENGTH = 15;
const int MAX_PASSWORD_LENGTH = 8;
const int MAX_SYSTEM_TYPE_LENGTH = 12;
const int MAX_ADDRESS_RANGE_LENGTH = 40;

class TestSmeProxy : public SmeProxy
{
	const SmeSystemId systemId;

public:
	TestSmeProxy(const SmeSystemId& id) : systemId(id) {}
	const SmeSystemId& getSystemId() const { return systemId; }
	
	//inherited
	virtual void putCommand(const SmscCommand& command) {}
	virtual void close() {}
	virtual uint32_t getNextSequenceNumber() { return 0; }
	virtual SmeProxyPriority getPriority() const { return 0; }
	virtual SmscCommand getCommand() { return SmscCommand(); }
	virtual SmeProxyState getState() const { return VALID; }
	virtual void init() {}
	virtual bool hasInput() const { return false; }
	virtual void attachMonitor(ProxyMonitor *monitor) {}
	virtual bool attached() { return false; }
};

}
}
}

#endif /* TEST_SMEMAN_SME_MANAGER_UTIL */

