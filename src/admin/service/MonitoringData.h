#ifndef SMSC_ADMIN_SERVICE_MONITORING_DATA
#define SMSC_ADMIN_SERVICE_MONITORING_DATA

#include <inttypes.h>
#include <core/buffers/Hash.hpp>

namespace smsc {
namespace admin {
namespace service {

using smsc::core::buffers::Hash;
using smsc::core::buffers::HashInvalidKeyException;

class MonitoringData : public Hash<int32_t>
{
public:
	void setParameter(const char * const name, int32_t value)
	{
		(*this)[name] = value;
	}
	int32_t getParameter(const char * const name)
		throw (HashInvalidKeyException &)
	{
		return Get(name);
	}

	char * getText() const;
};

}
}
}
#endif // ifndef SMSC_ADMIN_SERVICE_MONITORING_DATA
