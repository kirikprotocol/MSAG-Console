#ifndef SMSC_ADMIN_SERVICE_COMPONENT_MANAGER
#define SMSC_ADMIN_SERVICE_COMPONENT_MANAGER

#include <admin/AdminException.h>
#include <admin/service/Component.h>
#include <admin/service/Method.h>
#include <admin/service/Variant.h>
#include <core/synchronization/Mutex.hpp>

namespace smsc {
namespace admin {
namespace service {

using smsc::admin::AdminException;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

class ComponentManager
{
public:
	static Variant dispatch(const char * const componentName,
													const Method & method,
													const Arguments & args)
		throw (AdminException);

	static void registerComponent(Component * component)
		throw (AdminException);

	static void deregisterComponent(const char * const componentName)
		throw (AdminException);

	static const Components & getComponents() throw();

protected:
	static Components components;
	static Mutex componentsLock;
};

}
}
}

#endif // ifndef SMSC_ADMIN_SERVICE_COMPONENT_MANAGER
