#include "ComponentManager.h"

namespace smsc {
namespace admin {
namespace service {

Components ComponentManager::components;
Mutex ComponentManager::componentsLock;

Variant ComponentManager::dispatch(const char * const componentName,
																	 const Method & method,
																	 const Arguments & args)
	throw (AdminException)
{
	MutexGuard mutexGuard(componentsLock);
	Component * component = components[componentName];
	const Methods &methods(component->getMethods());

	if (!methods.Exists(method.getName()))
		throw AdminException("Unknown method");
	if (methods[method.getName()] != method)
		throw AdminException("Incorrect method signature");

	return component->call(methods[method.getName()], args);
}

void ComponentManager::registerComponent(Component * component)
	throw (AdminException)
{
	MutexGuard mutexGuard(componentsLock);
	if (component == 0 || component->getName() == 0)
		throw AdminException("Component or component name is null");
	components[component->getName()] = component;
}

void ComponentManager::deregisterComponent(const char * const componentName)
	throw (AdminException)
{
	MutexGuard mutexGuard(componentsLock);
	if (componentName == 0)
		throw AdminException("Component name is null");
	if (!components.Exists(componentName))
		throw AdminException("Component not found");
	components.Delete(componentName);
}

void ComponentManager::deregisterAllComponent()
{
	MutexGuard mutexGuard(componentsLock);
	components.Empty();
}

const Components & ComponentManager::getComponents() throw()
{
	return components;
}

}
}
}
