#ifndef SMSC_ADMIN_SMSC_SMSCCOMPONENT
#define SMSC_ADMIN_SMSC_SMSCCOMPONENT

#include <log4cpp/Category.hh>

#include <admin/service/Component.h>
#include <admin/service/Method.h>
#include <admin/service/Type.h>
#include <util/Logger.h>
#include <system/smsc.hpp>

namespace smsc {
namespace admin {
namespace smsc_service {

using smsc::system::SmscConfigs;
using smsc::admin::service::Method;
using smsc::admin::service::Methods;
using smsc::admin::service::Variant;
using smsc::admin::service::Arguments;
using smsc::admin::service::Parameters;
using smsc::admin::service::StringType;
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
		Method apply((unsigned)applyMethod, "apply", empty_params, StringType);

		methods[apply.getName()] = apply;

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
	
	bool isSmscRunning() throw() {return smsc_app_runner.get() != 0;}
	void apply() throw (AdminException);
	void reReadConfigs() throw (AdminException);

	SmscConfigs &configs;
	Methods methods;
	enum {applyMethod};

private:
	class SmscAppRunner : public Thread
	{
	public:
		SmscAppRunner(SmscConfigs &configs)
			: _app(new Smsc())
		{
			_app->init(configs);
		}
		virtual ~SmscAppRunner()
		{
			_app.reset(0);
		}
	
		virtual int Execute()
		{
			try{
				_app->run();
			}catch(std::exception& e)
			{
				fprintf(stderr,"top level exception: %s\n",e.what());
				return (-1);
			}
			catch(...)
			{
				fprintf(stderr,"FATAL EXCEPTION!\n");
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
	};

protected:
	std::auto_ptr<SmscAppRunner> smsc_app_runner;
	log4cpp::Category &logger;
};

}
}
}
#endif // ifndef SMSC_ADMIN_SMSC_SMSCCOMPONENT
