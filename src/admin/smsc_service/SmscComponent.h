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
    Parameters cancelMessage_params;
    cancelMessage_params["ids"] = Parameter("ids", StringType);
    cancelMessage_params["sources"] = Parameter("sources", StringType);
    cancelMessage_params["destinations"] = Parameter("destinations", StringType);

    Method apply_routes((unsigned)applyRoutesMethod, "apply_routes",
                        empty_params, StringType);
    Method apply_aliases((unsigned)applyAliasesMethod, "apply_aliases",
                         empty_params, StringType);
    Method lookup_profile((unsigned)lookupProfileMethod, "lookup_profile",
                          lookup_params, StringType);
    Method update_profile((unsigned)updateProfileMethod, "update_profile",
                          update_params, LongType);

    Method flush_statistics((unsigned)flushStatisticsMethod, "flush_statistics",
                      empty_params, StringType);
    Method process_cancel_messages((unsigned)processCancelMessagesMethod, "process_cancel_messages",
                          cancelMessage_params, StringType);
    Method apply_smsc_config((unsigned)applySmscConfigMethod, "apply_smsc_config",
                          empty_params, StringType);

    methods[apply_routes.getName()] = apply_routes;
    methods[apply_aliases.getName()] = apply_aliases;
    methods[lookup_profile.getName()] = lookup_profile;
    methods[update_profile.getName()] = update_profile;
    methods[flush_statistics.getName()] = flush_statistics;
    methods[process_cancel_messages.getName()] = process_cancel_messages;
    methods[apply_smsc_config.getName()] = apply_smsc_config;

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
  std::string flushStatistics(const Arguments &args) throw (AdminException);
  std::string processCancelMessages(const Arguments &args) throw (AdminException);
  void processCancelMessage(const std::string &sid, const std::string &ssrc, const std::string &sdst);

  bool isSmscRunning() throw() {return smsc_app_runner.get() != 0;}
  void applyRoutes() throw (AdminException);
  void applyAliases() throw (AdminException);
  void applySmscConfig() throw (AdminException);
  void reReadConfigs() throw (AdminException);

  SmscConfigs &configs;
  Methods methods;
  enum {applyRoutesMethod, applyAliasesMethod, lookupProfileMethod, updateProfileMethod, flushStatisticsMethod, processCancelMessagesMethod, applySmscConfigMethod};

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
        sigsend(P_PID, getpid(), smsc::admin::util::SignalHandler::SHUTDOWN_SIGNAL);
        return (-1);
      }
      catch(...)
      {
        fprintf(stderr,"FATAL EXCEPTION!\n");
        runner_logger.error("SMSC execution unknown exception.");
        sigsend(P_PID, getpid(), smsc::admin::util::SignalHandler::SHUTDOWN_SIGNAL);
        return (-0);
      }
      _app->shutdown();
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
