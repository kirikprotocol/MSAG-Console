#ifndef SMSC_IGORK_TESTS_DUMB_SERVICE_COMMAND_HANDLER
#define SMSC_IGORK_TESTS_DUMB_SERVICE_COMMAND_HANDLER

#include <admin/AdminException.h>
#include <admin/service/ServiceCommandHandler.h>
#include <admin/service/MonitoringData.h>
#include <util/config/Config.h>
#include <util/Logger.h>

namespace smsc {
namespace admin {
namespace service {
namespace test {

using smsc::admin::AdminException;
using smsc::admin::service::ServiceCommandHandler;
using smsc::admin::service::MonitoringData;
using smsc::util::Logger;
using smsc::util::config::Config;
using std::auto_ptr;

class DumbServiceCommandHandler : public ServiceCommandHandler
{
public:
	DumbServiceCommandHandler()
		: logger(Logger::getCategory("smsc.igork_tests.DumbServiceCommandHandler"))
	{
		mdata["aaa"]=5;
		mdata["test1"]=10;
		mdata["test2"]=20;
		mdata["test3"]=-30;
		config.setInt("aaa int 1", 1);
		config.setInt("bbb int -2", -2);
		config.setString("ccc string qwer", "qwer");
		config.setString("ddd string asdf", "asdf");
		config.setBool("eee bool true", true);
		config.setBool("fff bool false", false);
	}

	virtual Config & getConfig()
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
	Config config;
};

}
}
}
}

#endif
