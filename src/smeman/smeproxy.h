/*
	$Id$
*/

#include "util/debug.h"
#include "smsccmd.h"
#include "core/sycnhonization/eventmonitor.hpp"

#if !defined __Cpp_Header__smeman_smeproxy_h__
#define __Cpp_Header__smeman_smeproxy_h__

namespace smsc {
namespace smeman {

enum SmeProxyState
{
	VALID,
	INVALID
};

typedef int SmeProxyPriority;
typedef smsc::core::synchronization::Event ProxyMonitor;

// abstract
class SmeProxy
{
public:	
	//....
	void close() = 0;
	void putCommand(const SmscCommand& command) = 0;
	SmscCommand getCommand() = 0;
	SmeProxySate getState() const = 0;
	void init() = 0;
	SmeProxyPriority getPriority() const = 0;
	bool hasInput() const = 0;
	void attachMonitor(ProxyMonitor* monitor) = 0; // for detach monitor call with NULL
};

}; // namespace smeman
}; // namespace smsc

#endif


