#ifndef SMSC_IGORK_TESTS_DUMB_SERVICE_COMMAND_HANDLER
#define SMSC_IGORK_TESTS_DUMB_SERVICE_COMMAND_HANDLER

#include <admin/AdminException.h>
#include <admin/service/Component.h>
#include <admin/service/Method.h>
#include <admin/service/Parameter.h>
#include <admin/service/Type.h>
#include <admin/service/Variant.h>
#include <util/config/Config.h>
#include <logger/Logger.h>

namespace smsc {
namespace admin {
namespace service {
namespace test {

using smsc::admin::AdminException;
using namespace smsc::admin::service;
using smsc::util::config::Config;
using smsc::logger::Logger;

class DumbServiceCommandHandler : public Component
{
protected:
	Methods methods;
	enum MethodIds {
		getConfigId,
		getLogsId,
		getMonId,
		setConfigId
	};
	Config config;
	smsc::logger::Logger *logger;
	std::string dumb_config;

public:
	DumbServiceCommandHandler()
		: logger(Logger::getInstance("smsc.admin.service.test.DumbServiceCommandHandler"))
	{
		Parameters params_empty;
		methods["get service config"] = Method(getConfigId,
																					 "get service config",
																					 params_empty,
																					 StringType);
		methods["get service logs"] = Method(getLogsId,
																				 "get service logs",
																				 params_empty,
																				 StringType);
		methods["get service monitoring data"] = Method(getMonId,
																										"get service monitoring data",
																										params_empty,
																										StringType);
		Parameters params;
		params["config"] = Parameter("config", StringType);
		methods["set service config"] = Method(setConfigId,
																				 "set service config",
																				 params,
																				 StringType);
	}

	virtual ~DumbServiceCommandHandler() {}

	virtual const char * const getName() const
	{
		return "Dumb Service Component";
	}

	virtual const Methods & getMethods() const
	{
		return methods;
	}

	virtual Variant call(const Method & method, const Arguments & args)
		throw (AdminException)
	{
		switch(method.getId())
		{
		case getConfigId:
			{
				smsc_log_debug(logger, "getConfig");
				return Variant(dumb_config.c_str());
			}
		case getLogsId:
			smsc_log_debug(logger, "getLogs");
			return Variant("1014980394 INFO smsc.admin.service.ServiceSocketListener : Admin socket listener started on port 6677\n\
1014980424 DEBUG smsc.admin.service.CommandDispatcher : Command dispatcher \"192.168.2.72\" created.\n\
1014980424 DEBUG smsc.admin.service.CommandDispatcher : Command dispather starting...\n");
		case getMonId:
			smsc_log_debug(logger, "getMonitoring");
			return Variant("Monitoring Data");
		case setConfigId:
			return setConfig(args);
		default:
			smsc_log_debug(logger, "Unknown method");
			throw AdminException("Unknown method");
		}
	}

	Variant setConfig(const Arguments &params)
	{
		smsc_log_debug(logger, "setConfig:\n%s", params["config"].getStringValue());

		//setConfig
		dumb_config = params["config"].getStringValue();

		return Variant("Config setted sucessfully");
	}
};

}
}
}
}

#endif
