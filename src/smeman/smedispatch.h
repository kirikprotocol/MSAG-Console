/*
	$Id$
*/

#if !defined __Cpp_Header__smeman_smedispatch_h__
#define __Cpp_Header__smeman_smedispatch_h__
#include <inttypes.h>
#include <vector>
#include <list>
#include <queue>

#include "smetypes.h"
#include "smeproxy.h"
#include "core/synchronization/Mutex.hpp"

namespace smsc {
namespace smeman {
class SmeProxyDispatcher
{
	struct Unit
	{
		SmeProxyPriority prior;
		SmeProxy* proxy;
		Unit* next;
		Unit* prev;
	};
	Unit* queuedProxies;
	Unit* unqueuedProxies;
	ProxyMonitor mon;
	core::synchronization::Mutex dispatch_lock;
public:
	SmeProxyDispatcher(): queuedProxies(0),unqueuedProxies(0) {}
	SmeProxy* dispatchIn();
	void attachSmeProxy(SmeProxy* proxy);
	void detachSmeProxy(SmeProxy* proxy);
};

}; // namespace smeman
}; // namespace smsc

#endif


