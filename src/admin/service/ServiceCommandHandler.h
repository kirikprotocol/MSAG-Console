#ifndef	SMSC_ADMIN_SERVICE_SERVICE_COMMAND_HANDLER
#define SMSC_ADMIN_SERVICE_SERVICE_COMMAND_HANDLER

#include <inttypes.h>
#include <xercesc/dom/DOM_Node.hpp>
#include <admin/AdminException.h>
#include <admin/service/MonitoringData.h>
#include <core/buffers/Hash.hpp>
#include <util/config/Config.h>
#include <memory>

namespace smsc {
namespace admin {
namespace service {

using std::auto_ptr;
using smsc::util::config::Config;

class ServiceCommandHandler
{
public:
	virtual Config & getConfig()
			throw (AdminException &) = 0;
	virtual const char * const getLogs(uint32_t startLine, uint32_t lines)
			throw (AdminException &) = 0;
	virtual const MonitoringData& getMonitoring()
			throw (AdminException &) = 0;
	virtual void setConfig(Config & config)
			throw (AdminException &) = 0;
	virtual bool shutdown(void)
			throw (AdminException &) = 0;

protected:
private:
};

}
}
}
#endif // ifndef SMSC_ADMIN_SERVICE_SERVICE_COMMAND_HANDLER

