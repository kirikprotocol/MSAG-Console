#ifndef SMSC_ADMIN_SMSC_SMSCCOMPONENT
#define SMSC_ADMIN_SMSC_SMSCCOMPONENT

#include <admin/service/Component.h>
#include <admin/service/Method.h>
#include <admin/service/Type.h>
#include <core/synchronization/Mutex.hpp>
#include <logger/Logger.h>
#include <system/smsc.hpp>
#include "smeman/smeadmin.h"

namespace smsc {
namespace admin {
namespace smsc_service {

using smsc::system::SmscConfigs;
using smsc::system::Smsc;

using namespace smsc::admin::service;
using smsc::core::threads::Thread;
using smsc::admin::service::Component;
using smsc::logger::Logger;



using smsc::smeman::SmeAdministrator;

class SmscComponent : public Component
{
public:
  SmscComponent(SmscConfigs &all_configs, const char * node);
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
  void sigStopSmsc();
  bool isSmscRunning() {return smsc_app_runner.get() != 0;}
  bool isSmscStopping() {return isStopping;}
  void abort();
  void dump();

protected:
  Variant profileLookup(const Arguments &args) throw (AdminException);
  Variant profileLookupEx(const Arguments &args) throw (AdminException);
  int profileUpdate(const Arguments &args);
  int profileDelete(const Arguments &args);

  std::string flushStatistics(const Arguments &args) throw (AdminException);
  std::string processCancelMessages(const Arguments &args) throw (AdminException);
  void processCancelMessage(const std::string &sid, const std::string &ssrc, const std::string &sdst);

  void applyRoutes() throw (AdminException);
  void applyAliases() throw (AdminException);
  void applyReschedule() throw (AdminException);
  void applySmscConfig() throw (AdminException);
  void applyServices() throw (AdminException);
  Variant applyLocaleResource() throw (AdminException);
  void reloadConfigsAndRestart() throw (AdminException);
  void reReadConfigs() throw (AdminException);
  void applyTimeZones()throw(AdminException);
#ifdef USE_MAP
  void applyFraudControl()throw(AdminException);
  void applyMapLimits()throw(AdminException);
#endif

#ifdef SNMP
  void applySnmp();
#endif

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

  Variant loadRoutes(void) throw (AdminException);
  Variant traceRoute(const Arguments &args) throw (AdminException);

  Variant aclListNames(const Arguments & args) throw (AdminException);
  Variant aclGet(const Arguments & args) throw (AdminException);
  Variant aclRemove(const Arguments & args) throw (AdminException);
  Variant aclCreate(const Arguments & args) throw (AdminException);
  Variant aclUpdateInfo(const Arguments & args) throw (AdminException);
  Variant aclLookupAddresses(const Arguments & args) throw (AdminException);
  Variant aclRemoveAddresses(const Arguments & args) throw (AdminException);
  Variant aclAddAddresses(const Arguments & args) throw (AdminException);


  Variant dlListPrincipals(const Arguments & args) throw (AdminException);
  Variant dlAddPrincipal(const Arguments & args) throw (AdminException);
  Variant dlDelPrincipal(const Arguments & args) throw (AdminException);
  Variant dlGetPrincipal(const Arguments & args) throw (AdminException);
  Variant dlAlterPrincipal(const Arguments & args) throw (AdminException);

  Variant dlAddMember(const Arguments & args) throw (AdminException);
  Variant dlDeleteMember(const Arguments & args) throw (AdminException);
  Variant dlGetMembers(const Arguments & args) throw (AdminException);

  Variant dlAddSubmitter(const Arguments & args) throw (AdminException);
  Variant dlDeleteSubmitter(const Arguments & args) throw (AdminException);
  Variant dlListSubmitters(const Arguments & args) throw (AdminException);

  Variant dlAddList(const Arguments & args) throw (AdminException);
  Variant dlDeleteList(const Arguments & args) throw (AdminException);
  Variant dlGetList(const Arguments & args) throw (AdminException);
  Variant dlListLists(const Arguments & args) throw (AdminException);
  Variant dlAlterList(const Arguments & args) throw (AdminException);
  Variant dlCopyList(const Arguments & args) throw (AdminException);
  Variant dlRenameList(const Arguments & args) throw (AdminException);


  Variant setRole(const Arguments & args) throw (AdminException);
  Variant getRole() throw (AdminException);

  Variant cgmAddGroup(const Arguments & args);
  Variant cgmDeleteGroup(const Arguments & args);
  Variant cgmAddAddr(const Arguments & args);
  Variant cgmCheck(const Arguments & args);
  Variant cgmDelAddr(const Arguments & args);
  Variant cgmAddAbonent(const Arguments & args);
  Variant cgmDelAbonent(const Arguments & args);
  Variant cgmListAbonents(const Arguments & args);

  Variant addAlias(const Arguments & args);
  Variant delAlias(const Arguments & args);

#ifdef SMSEXTRA
  void setSponsoredValue(const Arguments& args);
#endif

//#ifdef SNMP
//  Variant snmpApply(const Arguments & args);
//#endif


  SmscConfigs &configs;
  Methods methods;
  enum
  {
    applyRoutesMethod, applyAliasesMethod, applyRescheduleMethod, applyServicesMethod, 
    applySmscConfigMethod, applyLocaleResourceMethod,applyTimeZonesMethod,applyFraudControlMethod,applyMapLimitsMethod,
    profileLookupMethod, profileUpdateMethod, profileLookupExMethod, profileDeleteMethod,
    flushStatisticsMethod,
    processCancelMessagesMethod,
    mscRegistrateMethod, mscUnregisterMethod, mscBlockMethod, mscClearMethod, mscListMethod,
    smeAddMethod, smeRemoveMethod, smeUpdateMethod, smeStatusMethod, smeDisconnectMethod,
    logGetCategoriesMethod, logSetCategoriesMethod,
    traceRouteMethod, loadRoutesMethod,
    aclListNamesMethod, aclGetMethod, aclRemoveMethod, aclCreateMethod, aclUpdateInfoMethod,
      aclLookupAddressesMethod, aclRemoveAddressesMethod, aclAddAddressesMethod,
    prcListPrincipalsMethod, prcAddPrincipalMethod, prcDeletePrincipalMethod, prcGetPrincipalMethod, prcAlterPrincipalMethod,
    memAddMemberMethod, memDeleteMemberMethod, memGetMemberMethod, sbmAddSubmiterMethod,
    sbmDeleteSubmiterMethod, sbmListSubmitersMethod,
    dlAddMethod, dlDeleteMethod, dlGetMethod, dlListMethod, dlAlterMethod,
    dlCopyMethod,dlRenameMethod,
    setRoleMethod, getRoleMethod,
    cgmAddGroupMethod,cgmDeleteGroupMethod,cgmAddAddrMethod,cgmCheckMethod,
    cgmDelAddrMethod,cgmAddAbonentMethod,cgmDelAbonentMethod,cgmListAbonentsMethod,
    aliasAddMethod,aliasDelMethod
#ifdef SNMP
    ,applySnmpMethod
#endif
#ifdef SMSEXTRA
    ,setSponsoredValuesMethod
#endif
  };

  smsc::core::synchronization::Mutex mutex;
    bool bTemporalRoutesManagerConfigLoaded;
  bool isStopping;

private:

  class SmscAppRunner : public Thread
  {
  public:
    SmscAppRunner(SmscConfigs &configs, const char * node)
    : _app(new Smsc()),
          runner_logger(Logger::getInstance("smsc.admin.smsc_service.SmscComponent.SmscAppRunner"))
    {
      _app->init(configs, node);
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
        smsc_log_error(runner_logger, "SMSC execution exception: \"%s\", SMSC stopped.", e.what());
        return(-1);
      }
      catch (...)
      {
        fprintf(stderr,"FATAL EXCEPTION!\n");
        smsc_log_error(runner_logger, "SMSC execution unknown exception.");
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
    smsc::logger::Logger *runner_logger;
  };

protected:
  std::auto_ptr<SmscAppRunner> smsc_app_runner;
  smsc::logger::Logger *logger;
  std::string node;
};

}
}
}
#endif // ifndef SMSC_ADMIN_SMSC_SMSCCOMPONENT
