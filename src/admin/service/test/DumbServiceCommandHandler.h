#ifndef SMSC_IGORK_TESTS_DUMB_SERVICE_COMMAND_HANDLER
#define SMSC_IGORK_TESTS_DUMB_SERVICE_COMMAND_HANDLER

#include <admin/AdminException.h>
#include <admin/service/Component.h>
#include <admin/service/Method.h>
#include <admin/service/Parameter.h>
#include <admin/service/Type.h>
#include <admin/service/Variant.h>
#include <util/config/Config.h>
#include <util/Logger.h>

namespace smsc {
namespace admin {
namespace service {
namespace test {

using smsc::admin::AdminException;
using smsc::admin::service;
using smsc::util::config::Config;
using smsc::util::Logger;

class DumbServiceCommandHandler : public Component
{
protected:
	Methods methods;
	enum MethodIds {getConfigId, getLogsId, getMonId, setConfigId};
	Config config;
	log4cpp::Category &logger;

public:
	DumbServiceCommandHandler()
		: logger(Logger::getCategory("smsc.admin.service.test.DumbServiceCommandHandler"))
	{
		Parameters params_empty;
		methods["getServiceConfig"] = Method(getConfigId,
																				 "getServiceConfig",
																				 params_empty,
																				 StringType);
		methods["getServiceLogs"] = Method(getLogsId,
																			 "getServiceLogs",
																			 params_empty,
																			 StringType);
		methods["getServiceMonitoringData"] = Method(getMonId,
																								 "getServiceMonitoringData",
																								 params_empty,
																								 StringType);
		Parameters params;
		params["config"] = Parameter("config", StringType);
		methods["setServiceConfig"] = Method(setConfigId,
																				 "setServiceConfig",
																				 params,
																				 StringType);
	}

	virtual const char * const getName() const
	{
		return "Dumb Service Component";
	}

	virtual const Methods & getMethods() const
	{
		return methods;
	}

	virtual Variant call(const Method & method, const Arguments & args)
		throw (AdminException &)
	{
		switch(method.getId())
		{
		case getConfigId:
			{
				logger.debug("getConfig");
				std::auto_ptr<char> conf(config.getTextRepresentation());
				return Variant(conf.get());
			}
		case getLogsId:
			logger.debug("getLogs");
			return Variant("1014980394 INFO smsc.admin.service.ServiceSocketListener : Admin socket listener started on port 6677\n\
1014980424 DEBUG smsc.admin.service.CommandDispatcher : Command dispatcher \"192.168.2.72\" created.\n\
1014980424 DEBUG smsc.admin.service.CommandDispatcher : Command dispather starting...\n");
		case getMonId:
			logger.debug("getMonitoring");
			return Variant("Monitoring Data");
		case setConfigId:
			return setConfig(args);
		default:
			logger.debug("Unknown method");
			throw AdminException("Unknown method");
		}
	}

	Variant setConfig(const Arguments &params)
	{
		logger.debug("setConfig:\n%s", params["config"].getStringValue());
		//setConfig
	}

/*	virtual Config & getConfig()
		throw(AdminException &)
	{
		logger.info("get config");
		return config;
	}

	virtual const char * const getLogs(uint32_t startLine, uint32_t lines)
		throw(AdminException &)
	{
		const char * const message =
"1014980394 INFO smsc.admin.service.ServiceSocketListener : Admin socket listener started on port 6677\n\
1014980424 DEBUG smsc.admin.service.CommandDispatcher : Command dispatcher \"192.168.2.72\" created.\n\
1014980424 DEBUG smsc.admin.service.CommandDispatcher : Command dispather starting...\n";
		logger.info("get logs");
		char * str = new char[std::strlen(message)+1];
		std::strcpy(str, message);
		return str;
	}

	virtual const MonitoringData& getMonitoring()
		throw(AdminException &)
	{
		logger.info("get monitoring");
		return mdata;
	}

	virtual void setConfig(Config &config)
		throw(AdminException &)
	{
		logger.info("set config");
	}

	virtual bool shutdown(void)
			throw (AdminException &)
	{
		logger.info("shutdown");
	}

protected:
	MonitoringData mdata;
	log4cpp::Category &logger;
*/
};

}
}
}
}

#endif
