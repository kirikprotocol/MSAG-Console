#ifndef SMSC_ADMIN_SMSC_SMSCCOMPONENT
#define SMSC_ADMIN_SMSC_SMSCCOMPONENT

#include <log4cpp/Category.hh>

#include <admin/service/Component.h>
#include <admin/service/Method.h>
#include <admin/service/Type.h>
#include <core/synchronization/Mutex.hpp>
#include <logger/Logger.h>
#include <system/smsc.hpp>

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
	SmscComponent(SmscConfigs &all_configs);
	virtual ~SmscComponent();

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
	void abort();
	void dump();

protected:
	int updateProfile(const Arguments &args);
	std::string lookupProfile(const Arguments &args) throw (AdminException);
	std::string flushStatistics(const Arguments &args) throw (AdminException);
	std::string processCancelMessages(const Arguments &args) throw (AdminException);
	void processCancelMessage(const std::string &sid, const std::string &ssrc, const std::string &sdst);

	bool isSmscRunning() throw()
	{
		return smsc_app_runner.get() != 0;
	}
	void applyRoutes() throw (AdminException);
	void applyAliases() throw (AdminException);
	void applySmscConfig() throw (AdminException);
	void applyServices() throw (AdminException);
	void reloadConfigsAndRestart() throw (AdminException);
	void reReadConfigs() throw (AdminException);

	void mscRegistrate(const Arguments & args);
	void mscUnregister(const Arguments & args);
	void mscBlock(const Arguments & args);
	void mscClear(const Arguments & args);
	Variant mscList();

	SmeAdministrator * getSmeAdmin();
	void smeAdd(const Arguments & args);
	void smeRemove(const Arguments & args);
	void smeUpdate(const Arguments & args);
	Variant smeStatus(const Arguments & args);
	void smeDisconnect(const Arguments & args);

	Variant logGetCategories(void);
	void logSetCategories(const Arguments & args);


	SmscConfigs &configs;
	Methods methods;
	enum
	{
		applyRoutesMethod, applyAliasesMethod, applyServicesMethod, applySmscConfigMethod,
		lookupProfileMethod, updateProfileMethod, 
		flushStatisticsMethod, 
		processCancelMessagesMethod,
		mscRegistrateMethod, mscUnregisterMethod, mscBlockMethod, mscClearMethod, mscListMethod,
		smeAddMethod, smeRemoveMethod, smeUpdateMethod, smeStatusMethod, smeDisconnectMethod,
		logGetCategoriesMethod, logSetCategoriesMethod
	};

	smsc::core::synchronization::Mutex mutex;
	bool isStopping;

private:
	void setLogCat(const char * catStr);

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
			try
			{
				_app->run();
			}
			catch (std::exception& e)
			{
				fprintf(stderr,"top level exception: %s\n",e.what());
				runner_logger.error("SMSC execution exception: \"%s\", SMSC stopped.", e.what());
				return(-1);
			}
			catch (...)
			{
				fprintf(stderr,"FATAL EXCEPTION!\n");
				runner_logger.error("SMSC execution unknown exception.");
				return(-0);
			}
			_app->shutdown();
			fprintf(stderr,"SMSC finished\n");
			return 0;
		}

		void stop()
		{
			_app->stop();
		}

		void abort()
		{
			_app->abortSmsc();
		}

		void dump()
		{
			_app->dumpSmsc();
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

