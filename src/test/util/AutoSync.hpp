#ifndef TEST_UTIL_AUTO_SYNC
#define TEST_UTIL_AUTO_SYNC

#include "core/synchronization/Mutex.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace util {

template<class T>
class auto_sync
{
	T* obj;

	auto_sync(const auto_sync&) {}
	auto_sync<T>& operator=(const auto_sync<T>& as) {}

public:
	auto_sync() : obj(NULL) {}

	auto_sync(T* o) : obj(o)
	{
		if (obj)
		{
			//__trace2__("auto_sync(T*): lock, this=%p, obj=%p", this, obj);
			obj->getMutex().Lock();
		}
	}

	~auto_sync()
	{
		if (obj)
		{
			//__trace2__("~auto_sync(): unlock, this=%p, obj=%p", this, obj);
			obj->getMutex().Unlock();
		}
	}

	T* get() { return obj; }

	T& operator*() { return *obj; }
	T* operator->() { return obj; }

	auto_sync<T>& operator=(T* o)
	{
		if (obj)
		{
			//__trace2__("auto_sync:operator=(T*): unlock, this=%p, obj=%p", this, obj);
			obj->getMutex().Unlock();
		}
		obj = o;
		if (obj)
		{
			//__trace2__("auto_sync:operator=(T*): lock, this=%p, obj=%p", this, obj);
			obj->getMutex().Lock();
		}
		return *this;
	}
};

}
}
}

#endif /* TEST_UTIL_AUTO_SYNC */
