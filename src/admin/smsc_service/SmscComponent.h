#ifndef SMSC_ADMIN_SMSC_SMSCCOMPONENT
#define SMSC_ADMIN_SMSC_SMSCCOMPONENT

#include <log4cpp/Category.hh>

#include <admin/service/Component.h>
#include <admin/service/Method.h>
#include <admin/service/Type.h>
#include <core/synchronization/Mutex.hpp>
#include <util/Logger.h>
#include <system/smsc.hpp>
#include <admin/util/SignalHandler.h>
#include <signal.h>

namespace smsc {
namespace admin {
namespace smsc_service {

using smsc::system::SmscConfigs;
using namespace smsc::admin::service;
using smsc::core::threads::Thread;
using smsc::system::Smsc;
using smsc::admin::service::Component;
using smsc::util::Logger;

class SmscComponent : public Component
{
public:
	SmscComponent(SmscConfigs &all_configs)
		: configs(all_configs),
		  logger(Logger::getCategory("smsc.admin.smsc_service.SmscComponent"))
	{
		Parameters empty_params;
    Parameters lookup_params;
    lookup_params["address"] = Parameter("address", StringType);
    Parameters update_params;
    update_params["address"] = Parameter("address", StringType);
    update_params["profile"] = Parameter("profile", StringType);

		Method apply_routes((unsigned)applyRoutesMethod, "apply_routes",
                        empty_params, StringType);
		Method apply_aliases((unsigned)applyAliasesMethod, "apply_aliases",
                         empty_params, StringType);
    Method lookup_profile((unsigned)lookupProfileMethod, "lookup_profile",
                          lookup_params, StringType);
    Method update_profile((unsigned)updateProfileMethod, "update_profile",
                          update_params, LongType);

		methods[apply_routes.getName()] = apply_routes;
		methods[apply_aliases.getName()] = apply_aliases;
    methods[lookup_profile.getName()] = lookup_profile;
    methods[update_profile.getName()] = update_profile;

		smsc_app_runner.reset(0);
	}
	virtual ~SmscComponent()
	{
		smsc_app_runner.reset(0);
	}

	virtual const char * const getName() const
	{
		return "SMSC";
	}

	virtual const Methods & getMethods() const
	{
		return methods;
	}

	virtual Variant call(const Method & method, const Arguments & args)
		throw (AdminException);

	void runSmsc() throw (AdminException);

	void stopSmsc() throw (AdminException);

protected:
  int updateProfile(const Arguments &args);
  std::string lookupProfile(const Arguments &args) throw (AdminException);
	
	bool isSmscRunning() throw() {return smsc_app_runner.get() != 0;}
	void applyRoutes() throw (AdminException);
	void applyAliases() throw (AdminException);
	void reReadConfigs() throw (AdminException);

	SmscConfigs &configs;
	Methods methods;
	enum {applyRoutesMethod, applyAliasesMethod, lookupProfileMethod, updateProfileMethod};

  smsc::core::synchronization::Mutex mutex;

private:
	class SmscAppRunner : public Thread
	{
	public:
		SmscAppRunner(SmscConfigs &configs)
			: _app(new Smsc()),
        runner_logger(Logger::getCategory("smsc.admin.smsc_service.SmscComponent.SmscAppRunner"))
		{
			_app->init(configs);
		}
		virtual ~SmscAppRunner()
		{
			_app.reset(0);
		}

        Smsc * getApp()
        {
            return _app.get();
        }
	
		virtual int Execute()
		{
			try{
				_app->run();
			}catch(std::exception& e)
			{
				fprintf(stderr,"top level exception: %s\n",e.what());
        runner_logger.error("SMSC execution exception: \"%s\", SMSC stopped.", e.what());
        //_app->shutdown();
        sigsend(P_PID, getpid(), smsc::admin::util::SignalHandler::SHUTDOWN_SIGNAL);
				return (-1);
			}
			catch(...)
			{
				fprintf(stderr,"FATAL EXCEPTION!\n");
        runner_logger.error("SMSC execution unknown exception.");
        //_app->shutdown();
        sigsend(P_PID, getpid(), smsc::admin::util::SignalHandler::SHUTDOWN_SIGNAL);
				return (-0);
			}
	
			fprintf(stderr,"SMSC finished\n");
			return 0;
		}

		void stop()
		{
			_app->stop();
		}
	
	protected:
		std::auto_ptr<Smsc> _app;
    log4cpp::Category &runner_logger;
	};

protected:
	std::auto_ptr<SmscAppRunner> smsc_app_runner;
	log4cpp::Category &logger;
};

}
}
}
#endif // ifndef SMSC_ADMIN_SMSC_SMSCCOMPONENT
