#ifndef SMSC_ADMIN_SERVICE_COMPONENT
#define SMSC_ADMIN_SERVICE_COMPONENT

#include <admin/AdminException.h>
#include <admin/service/Method.h>
#include <admin/service/Variant.h>
#include <core/buffers/Hash.hpp>

namespace smsc {
namespace admin {
namespace service {

using smsc::core::buffers::Hash;

class Component
{
public:
	virtual const char * const getName() const = 0;
	virtual const Methods & getMethods() const = 0;
	virtual Variant call(const Method & method, const Arguments & args)
		throw (AdminException) = 0;
};

typedef Hash<Component*> Components;

}
}
}
#endif // ifndef SMSC_ADMIN_SERVICE_COMPONENT
