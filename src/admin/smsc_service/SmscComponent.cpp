#include "SmscComponent.h"
#include "profiler/profiler.hpp"

#include <system/smsc.hpp>

#include <util/config/Manager.h>
#include <util/config/route/RouteConfig.h>
#include <mscman/MscManager.h>
#include <resourcemanager/ResourceManager.hpp>

namespace smsc {
namespace admin {
namespace smsc_service {

using namespace smsc::system;
using namespace smsc::profiler;
using namespace smsc::core::buffers;
using namespace smsc::util::config;
using namespace smsc::util::config::route;

using smsc::mscman::MscManager;
using smsc::mscman::MscInfo;

SmscComponent::SmscComponent(SmscConfigs &all_configs)
: configs(all_configs),isStopping(false),
      logger(Logger::getInstance("smsc.admin.smsc_service.SmscComponent"))
{
  /*********************** parameters ***************************************/
  Parameters empty_params;
  Parameters lookup_params;
  lookup_params["address"] = Parameter("address", StringType);
  Parameters update_params;
  update_params["address"] = Parameter("address", StringType);
  update_params["profile"] = Parameter("profile", StringListType);
  Parameters cancelMessage_params;
  cancelMessage_params["ids"] = Parameter("ids", StringType);
  cancelMessage_params["sources"] = Parameter("sources", StringType);
  cancelMessage_params["destinations"] = Parameter("destinations", StringType);
  Parameters msc_params;
  msc_params["msc"] = Parameter("msc", StringType);

  Parameters sme_params;
  sme_params["id"                ] = Parameter("id"                , StringType);
  sme_params["priority"          ] = Parameter("priority"          , LongType);
  sme_params["typeOfNumber"      ] = Parameter("typeOfNumber"      , LongType);
  sme_params["numberingPlan"     ] = Parameter("numberingPlan"     , LongType);
  sme_params["interfaceVersion"  ] = Parameter("interfaceVersion"  , LongType);
  sme_params["systemType"        ] = Parameter("systemType"        , StringType);
  sme_params["password"          ] = Parameter("password"          , StringType);
  sme_params["addrRange"         ] = Parameter("addrRange"         , StringType);
  sme_params["smeN"              ] = Parameter("smeN"              , LongType);
  sme_params["wantAlias"         ] = Parameter("wantAlias"         , BooleanType);
  sme_params["forceDC"           ] = Parameter("forceDC"           , BooleanType);
  sme_params["timeout"           ] = Parameter("timeout"           , LongType);
  sme_params["receiptSchemeName" ] = Parameter("receiptSchemeName" , StringType);
  sme_params["disabled"          ] = Parameter("disabled"          , BooleanType);
  sme_params["mode"              ] = Parameter("mode"              , StringType);
  sme_params["proclimit"         ] = Parameter("proclimit"         , LongType);
  sme_params["schedlimit"        ] = Parameter("schedlimit"        , LongType);
  Parameters sme_id_params;
  sme_id_params["id"] = Parameter("id", StringType);
  Parameters sme_ids_params;
  sme_ids_params["ids"] = Parameter("ids", StringListType);
  Parameters log_cats;
  log_cats["categories"] = Parameter("categories", StringListType);


  /**************************** method declarations *************************/
  Method apply_routes          ((unsigned)applyRoutesMethod,         "apply_routes",          empty_params, StringType);
  Method apply_aliases         ((unsigned)applyAliasesMethod,        "apply_aliases",         empty_params, StringType);
  Method apply_smsc_config     ((unsigned)applySmscConfigMethod,     "apply_smsc_config",     empty_params, StringType);
  Method apply_services        ((unsigned)applyServicesMethod,       "apply_services",        empty_params, StringType);
  Method apply_locale_resource ((unsigned)applyLocaleResourceMethod, "apply_locale_resources",empty_params, StringType);

  Parameters trace_route_params;
  trace_route_params["dstAddress"] = Parameter("dstAddress", StringType);
  trace_route_params["srcAddress"] = Parameter("srcAddress", StringType);
  trace_route_params["srcSysId"  ] = Parameter("srcSysId"  , StringType);
  Method trace_route           ((unsigned)traceRouteMethod, "trace_route", trace_route_params, StringListType);
  Method load_routes           ((unsigned)loadRoutesMethod, "load_routes", empty_params, StringListType);
    bTemporalRoutesManagerConfigLoaded = false;

  Method profile_lookup   ((unsigned)profileLookupMethod,   "lookup_profile",    lookup_params, StringListType);
  Method profile_lookup_ex((unsigned)profileLookupExMethod, "profile_lookup_ex", lookup_params, StringListType);
  Method profile_update   ((unsigned)profileUpdateMethod,   "update_profile",    update_params, LongType);
  Method profile_delete   ((unsigned)profileDeleteMethod,   "profile_delete",    lookup_params, LongType);

  Method flush_statistics       ((unsigned)flushStatisticsMethod,       "flush_statistics",        empty_params,         StringType);
  Method process_cancel_messages((unsigned)processCancelMessagesMethod, "process_cancel_messages", cancelMessage_params, StringType);

  Method msc_registrate((unsigned)mscRegistrateMethod, "msc_registrate", msc_params,   StringType);
  Method msc_unregister((unsigned)mscUnregisterMethod, "msc_unregister", msc_params,   StringType);
  Method msc_block     ((unsigned)mscBlockMethod,      "msc_block",      msc_params,   StringType);
  Method msc_clear     ((unsigned)mscClearMethod,      "msc_clear",      msc_params,   StringType);
  Method msc_list      ((unsigned)mscListMethod,       "msc_list",       empty_params, StringListType);

  Method sme_add        ((unsigned)smeAddMethod,        "sme_add",         sme_params,     BooleanType);
  Method sme_remove     ((unsigned)smeRemoveMethod,     "sme_remove",      sme_id_params,  BooleanType);
  Method sme_update     ((unsigned)smeUpdateMethod,     "sme_update",      sme_params,     BooleanType);
  Method sme_status     ((unsigned)smeStatusMethod,     "sme_status",      empty_params,   StringListType);
  Method sme_disconnect ((unsigned)smeDisconnectMethod, "sme_disconnect",  sme_ids_params, BooleanType);

  Method log_get_categories((unsigned)logGetCategoriesMethod, "log_get_categories",  empty_params, StringListType);
  Method log_set_categories((unsigned)logSetCategoriesMethod, "log_set_categories",  log_cats, BooleanType);

  /***************************** method assigns *****************************/
  methods[apply_routes         .getName()] = apply_routes;
  methods[apply_aliases        .getName()] = apply_aliases;
  methods[apply_smsc_config    .getName()] = apply_smsc_config;
  methods[apply_services       .getName()] = apply_services;
  methods[apply_locale_resource.getName()] = apply_locale_resource;

  methods[profile_lookup.getName()] = profile_lookup;
  methods[profile_update.getName()] = profile_update;
  methods[profile_lookup_ex.getName()] = profile_lookup_ex;
  methods[profile_delete.getName()] = profile_delete;

  methods[flush_statistics       .getName()] = flush_statistics;
  methods[process_cancel_messages.getName()] = process_cancel_messages;

  methods[msc_registrate.getName()] = msc_registrate;
  methods[msc_unregister.getName()] = msc_unregister;
  methods[msc_block     .getName()] = msc_block;
  methods[msc_clear     .getName()] = msc_clear;
  methods[msc_list      .getName()] = msc_list;

  methods[sme_add       .getName()] = sme_add;
  methods[sme_remove    .getName()] = sme_remove;
  methods[sme_update    .getName()] = sme_update;
  methods[sme_status    .getName()] = sme_status;
  methods[sme_disconnect.getName()] = sme_disconnect;

  methods[log_get_categories.getName()] = log_get_categories;
  methods[log_set_categories.getName()] = log_set_categories;

  methods[load_routes.getName()] = load_routes;
  methods[trace_route.getName()] = trace_route;

  smsc_app_runner.reset(0);
}

SmscComponent::~SmscComponent()
{
  smsc_app_runner.reset(0);
}



Variant SmscComponent::call(const Method & method, const Arguments & args)
throw (AdminException)
{
  try
  {
    logger.debug("call \"%s\"", method.getName());
    switch (method.getId())
    {
      case applyRoutesMethod:
        logger.debug("applying routes...");
        applyRoutes();
        logger.debug("routes applied");
        return Variant("");
      case applyAliasesMethod:
        logger.debug("applying aliases...");
        applyAliases();
        logger.debug("aliases applied");
        return Variant("");
      case profileLookupMethod:
        logger.debug("lookup profile...");
        return profileLookup(args);
      case profileLookupExMethod:
        logger.debug("lookupEx profile...");
        return profileLookupEx(args);
      case profileUpdateMethod:
        logger.debug("update profile...");
        return Variant((long)profileUpdate(args));
      case profileDeleteMethod:
        return Variant((long)profileDelete(args));
      case flushStatisticsMethod:
        return Variant(flushStatistics(args).c_str());
      case processCancelMessagesMethod:
        return Variant(processCancelMessages(args).c_str());
      case applySmscConfigMethod:
        applySmscConfig();
        return Variant("");
      case applyServicesMethod:
        applyServices();
        return Variant("");
      case applyLocaleResourceMethod:
        return applyLocaleResource();


      case mscRegistrateMethod:
        mscRegistrate(args);
        return Variant("");
      case mscUnregisterMethod:
        mscUnregister(args);
        return Variant("");
      case mscBlockMethod:
        mscBlock(args);
        return Variant("");
      case mscClearMethod:
        mscClear(args);
        return Variant("");
      case mscListMethod:
        return mscList();

      case smeAddMethod:
        smeAdd(args);
        return Variant(true);
      case smeRemoveMethod:
        smeRemove(args);
        return Variant(true);
      case smeUpdateMethod:
        smeUpdate(args);
        return Variant(true);
      case smeStatusMethod:
        return smeStatus(args);
      case smeDisconnectMethod:
        smeDisconnect(args);
        return Variant(true);
      case logGetCategoriesMethod:
        return logGetCategories();
      case logSetCategoriesMethod:
        logSetCategories(args);
        return Variant(true);
      case loadRoutesMethod:
                return loadRoutes();
      case traceRouteMethod:
                return traceRoute(args);

      default:
        logger.debug("unknown method \"%s\" [%u]", method.getName(), method.getId());
        throw AdminException("Unknown method \"%s\"", method.getName());
    }
    logger.error("call \"%s\"[%u] done. Unreacheable code reached.", method.getName(), method.getId());
    return Variant("");
  }
  catch (AdminException &e)
  {
    throw e;
  }
  catch (...)
  {
    throw AdminException("Unknown exception catched during call");
  }
}

void SmscComponent::mscRegistrate(const Arguments & args)
{
  MscManager::getMscAdmin().registrate(args.Get("msc").getStringValue());
}
void SmscComponent::mscUnregister(const Arguments & args)
{
  MscManager::getMscAdmin().unregister(args.Get("msc").getStringValue());
}
void SmscComponent::mscBlock(const Arguments & args)
{
  MscManager::getMscAdmin().block(args.Get("msc").getStringValue());
}
void SmscComponent::mscClear(const Arguments & args)
{
  MscManager::getMscAdmin().clear(args.Get("msc").getStringValue());
}
Variant SmscComponent::mscList()
{
  const Array<MscInfo> & mscInfos = MscManager::getMscAdmin().list();
  Variant result(StringListType);
  for (int i=0; i<mscInfos.Count(); i++)
  {
    char buffer[256];
    snprintf(buffer, 255, "%i", mscInfos[i].failureCount);
    std::string  buf;
    buf += mscInfos[i].mscNum;
    buf += ',';
    buf += mscInfos[i].manualLock ? "true" : "false";
    buf += ',';
    buf += mscInfos[i].automaticLock ? "true" : "false";
    buf += ',';
    buf += buffer;
    result.appendValueToStringList(buf.c_str());
  }
  return result;
/*              Variant result(StringListType);
result.appendValueToStringList("test1");
result.appendValueToStringList("test2");
result.appendValueToStringList("test3");
result.appendValueToStringList("a1");
result.appendValueToStringList("a3");
result.appendValueToStringList("a2");
return result;*/
}


void SmscComponent::applyServices()
throw (AdminException)
{
  logger.info("applying new services...");
  reloadConfigsAndRestart();
  logger.info("new services applied.");
}

void SmscComponent::applySmscConfig()
throw (AdminException)
{
  logger.info("applying new configs...");
  reloadConfigsAndRestart();
  logger.info("new config applied.");
}

void SmscComponent::reloadConfigsAndRestart()
throw (AdminException)
{
  try
  {
    stopSmsc();
    reReadConfigs();
    runSmsc();
  }
  catch (std::exception &e)
  {
    logger.error("Couldn't apply new config: %s", e.what());
    return;
  }
  catch (...)
  {
    logger.error("Couldn't apply new config: unknown exception");
    return;
  }
}

std::string SmscComponent::flushStatistics(const Arguments &args)
throw (AdminException)
{
  logger.debug("flushStatistics");
  try
  {
    smsc_app_runner->getApp()->flushStatistics();
  }
  catch (std::exception &e)
  {
    logger.error("Exception on flush statistics: %s", e.what());
    throw AdminException("Exception on flush statistics");
  }
  catch (...)
  {
    logger.error("Unknown exception on flush statistics");
    throw AdminException("Unknown exception on flush statistics");
  }
  return "";
}

typedef Array<std::string> StrArray;
StrArray parseStringToStringList(const char * const strlist)
{
  StrArray result;
  const char * str = strlist;
  for (const char * ptr = strchr(str, ','); ptr != 0; ptr = strchr(str, ','))
  {
    while (str[0] == ' ') str++;
    const int length = ptr-str;
    char * value_ = new char[length+1];
    memcpy(value_, str, length*sizeof(char));
    value_[length] = 0;
    std::string value(value_);
    result.Push(value);
    str = ptr+1;
  }
  if (strlen(str) > 0)
  {
    while (str[0] == ' ') str++;
    result.Push(str);
  }
  return result;
}

void SmscComponent::processCancelMessage(const std::string &sid, const std::string &ssrc, const std::string &sdst)
{
  try
  {
    SMSId id = 0;
    sscanf(sid.c_str(), "%llx", &id);
    Address src(ssrc.c_str());
    Address dst(sdst.c_str());
#ifdef SMSC_DEBUG
    /*
    char src_[16+src.getLength()];
    src.toString(src_, sizeof(src_)/sizeof(char));
    char dst_[16+src.getLength()];
    dst.toString(dst_, sizeof(dst_)/sizeof(char));
    __trace2__("processCancelMessage: %llx[%s][%s]\n", id, src_, dst_);
    */
#endif
    smsc_app_runner->getApp()->cancelSms(id, src, dst);
    logger.info("message %s[%s][%s] canceled", sid.c_str(), ssrc.c_str(), sdst.c_str());
  }
  catch (...)
  {
    logger.error("Couldn't cancel message %s[%s][%s]", sid.c_str(), ssrc.c_str(), sdst.c_str());
  }
}

std::string SmscComponent::processCancelMessages(const Arguments &args)
throw (AdminException)
{
  try
  {
    const char * const idsStr = args.Get("ids").getStringValue();
    const char * const sourcesStr = args.Get("sources").getStringValue();
    const char * const destinationsStr = args.Get("destinations").getStringValue();
    logger.info("processCancelMessages: [%s], [%s], [%s]", idsStr, sourcesStr, destinationsStr);

    StrArray ids = parseStringToStringList(idsStr);
    StrArray sources = parseStringToStringList(sourcesStr);
    StrArray destinations = parseStringToStringList(destinationsStr);
    if ((ids.Count() != sources.Count())
       || (ids.Count() != destinations.Count())
       || (destinations.Count() != sources.Count()))
    {
      logger.error("processCancelMessages failed: incorrect params");
      throw AdminException("incorrect params");
    }

    for (int i = 0; i < ids.Count(); i++)
    {
      std::string sid(ids[i]);
      std::string ssrc(sources[i]);
      std::string sdst(destinations[i]);

      processCancelMessage(sid, ssrc, sdst);
    }

    return "";
  }
  catch (HashInvalidKeyException &e)
  {
    logger.error("processCancelMessages failed: not all parameters defined");
    throw AdminException("not all parameters defined");
  }
  catch (...)
  {
    logger.error("processCancelMessages failed: unknown reason");
    throw AdminException("unknown exception");
  }
}

void SmscComponent::runSmsc()
throw (AdminException)
{
  smsc::core::synchronization::MutexGuard guard(mutex);
  if (smsc_app_runner.get() == 0)
  {
    try
    {
      smsc_app_runner.reset(new SmscAppRunner(configs));
      smsc::resourcemanager::ResourceManager::reload(configs.cfgman->getString("core.locales"), configs.cfgman->getString("core.default_locale"));
      smsc_app_runner->Start();
    }
    catch (smsc::util::Exception &e)
    {
      logger.error("Exception on starting SMSC: \"%s\"", e.what());
      throw AdminException("Exception on starting SMSC: \"%s\"", e.what());
    }
    catch (std::exception &e)
    {
      logger.error("Exception on starting SMSC: \"%s\"", e.what());
      throw AdminException("Exception on starting SMSC: \"%s\"", e.what());
    }
    catch (...)
    {
      logger.error("Unknown exception on starting SMSC");
      throw AdminException("Unknown exception on starting SMSC");
    }
  }
  else
    throw   AdminException("SMSC Application started already (or not sucessfully stopped)");
}

void SmscComponent::stopSmsc()
throw (AdminException)
{
  if (isStopping) return;
  smsc::core::synchronization::MutexGuard guard(mutex);
  isStopping = true;
  if (smsc_app_runner.get() != 0)
  {
    try
    {
      smsc_app_runner->stop();
      smsc_app_runner->WaitFor();
      smsc_app_runner.reset(0);
      isStopping = false;
    }
    catch (smsc::util::Exception &e)
    {
      isStopping = false;
      logger.error("Exception on starting SMSC: \"%s\"", e.what());
      throw AdminException("Exception on stopping SMSC: \"%s\"", e.what());
    }
    catch (std::exception &e)
    {
      isStopping = false;
      logger.error("Exception on stopping SMSC: \"%s\"", e.what());
      throw AdminException("Exception on starting SMSC: \"%s\"", e.what());
    }
    catch (...)
    {
      isStopping = false;
      logger.error("Unknown exception on stopping SMSC");
      throw AdminException("Unknown exception on starting SMSC");
    }
  }
  else
    throw   AdminException("SMSC Application stopped already");
}

void SmscComponent::abort()
{
//                smsc::core::synchronization::MutexGuard guard(mutex);
  if (smsc_app_runner.get() != 0)
  {
    try
    {
      smsc_app_runner->abort();
      smsc_app_runner->WaitFor();
      smsc_app_runner.reset(0);
    }
    catch (smsc::util::Exception &e)
    {
      logger.error("Exception on aborting SMSC: \"%s\"", e.what());
      throw AdminException("Exception on aborting SMSC: \"%s\"", e.what());
    }
    catch (std::exception &e)
    {
      logger.error("Exception on aborting SMSC: \"%s\"", e.what());
    }
    catch (...)
    {
      logger.error("Unknown exception on aborting SMSC");
    }
  }
}

void SmscComponent::dump()
{
//                smsc::core::synchronization::MutexGuard guard(mutex);
  if (smsc_app_runner.get() != 0)
  {
    try
    {
      smsc_app_runner->dump();
      smsc_app_runner->WaitFor();
      smsc_app_runner.reset(0);
    }
    catch (smsc::util::Exception &e)
    {
      logger.error("Exception on starting SMSC: \"%s\"", e.what());
      throw AdminException("Exception on dumping SMSC: \"%s\"", e.what());
    }
    catch (std::exception &e)
    {
      logger.error("Exception on dumping SMSC: \"%s\"", e.what());
    }
    catch (...)
    {
      logger.error("Unknown exception on dumping SMSC");
    }
  }
}

void SmscComponent::applyRoutes()
throw (AdminException)
{
  configs.routesconfig->reload();
  configs.smemanconfig->reload();
  smsc_app_runner->getApp()->reloadRoutes(configs);
}

Variant SmscComponent::loadRoutes(void)
    throw (AdminException)
{
    try
    {
        bTemporalRoutesManagerConfigLoaded = false;
        RouteConfig cfg;
        if (cfg.load("conf/routes__.xml") == RouteConfig::fail)
            throw AdminException("Load routes config file failed.");

        vector<std::string> traceBuff;
        smsc_app_runner->getApp()->reloadTestRoutes(cfg);
        smsc_app_runner->getApp()->getTestRouterInstance()->enableTrace(true);
        smsc_app_runner->getApp()->getTestRouterInstance()->getTrace(traceBuff);

        // 0:   Message (Routes successfully loaded)
        // 1..: Trace (if any)

        Variant result(service::StringListType);
        result.appendValueToStringList("Routes configuration successfully loaded");

        for (int i=0; i<traceBuff.size(); i++)
            result.appendValueToStringList(traceBuff[i].c_str());

        bTemporalRoutesManagerConfigLoaded = true;
        return result;
    }
    catch (AdminException& aexc) {
        throw;
    }
    catch (ConfigException& cexc) {
        throw AdminException("Load routes config file failed. Cause: %s", cexc.what());
    }
    catch (std::exception& exc) {
        throw AdminException("Load routes failed. Cause: %s.", exc.what());
    }
    catch (...) {
        throw AdminException("Load routes failed. Cause is unknown.");
    }
}

const char* getStringParameter(const Arguments &args, const char* param)
{
    const char* value = args.Get(param).getStringValue();
    return (value && value[0] == '\0') ? 0:value;
}

inline const size_t getEncodedStringSize(const char* const src)
{
    size_t count = 0;
    for (const char *p = src; *p != 0; p++)
        count += (*p == '#' || *p == ':' || *p == ';') ? 2:1;
    return count;
}
inline char* getEncodedString(const char* const src)
{
    char* result = new char[getEncodedStringSize(src)+1];
    char* d = result;
    for (const char *s = src; *s != 0; s++)
    {
        switch(*s)
        {
            case '#': *d++='#'; *d++='#'; break;
            case ':': *d++='#'; *d++='c'; break;
            case ';': *d++='#'; *d++='s'; break;
            default : *d++=*s;
        }
    }
    *d='\0';
    return result;
}

Variant SmscComponent::traceRoute(const Arguments &args)
    throw (AdminException)
{
    const char* dstAddr  = getStringParameter(args, "dstAddress");
    const char* srcAddr  = getStringParameter(args, "srcAddress");
    const char* srcSysId = getStringParameter(args, "srcSysId");

    try
    {
        if (!bTemporalRoutesManagerConfigLoaded)
        {
            RouteConfig cfg;
            if (cfg.load("conf/routes__.xml") == RouteConfig::fail)
                throw AdminException("Load routes config file failed.");

            vector<std::string> traceBuff;
            smsc_app_runner->getApp()->reloadTestRoutes(cfg);
            smsc_app_runner->getApp()->getTestRouterInstance()->enableTrace(true);
            smsc_app_runner->getApp()->getTestRouterInstance()->getTrace(traceBuff);

            bTemporalRoutesManagerConfigLoaded = true;
        }

        SmeProxy* proxy = 0;
        RouteInfo info;
        bool found = false;
        info.enabling = true;

        //-1:   Dealiased destination
        // 0:   Message (Route found | Route found (disabled) | Route not found)
        // 1:   RouteInfo (if any)
        // 2..: Trace (if any)

        Variant result(service::StringListType);


        Address dealiased;
        char addrBuf[MAX_ADDRESS_VALUE_LENGTH+5];
        string dealiasText="There are no aliases for this address";
        if(smsc_app_runner->getApp()->AliasToAddress(Address(dstAddr),dealiased))
        {
          dealiasText="Address "+Address(dstAddr).toString()+" was dealiased to "+dealiased.toString();
          dealiased.toString(addrBuf,sizeof(addrBuf));
          dstAddr=addrBuf;
        }

        if (srcSysId)
        {
            SmeIndex index = smsc_app_runner->getApp()->getSmeIndex(srcSysId);
            if (index == -1)
                throw AdminException("Trace route failed. Sme for system id '%s' not found", srcSysId);

            found = smsc_app_runner->getApp()->getTestRouterInstance()->
                lookup(index, Address(srcAddr), Address(dstAddr), proxy, 0, &info);
        }
        else
        {
            found = smsc_app_runner->getApp()->getTestRouterInstance()->
                lookup(Address(srcAddr), Address(dstAddr), proxy, 0, &info);
        }

        vector<std::string> traceBuff;
        smsc_app_runner->getApp()->getTestRouterInstance()->getTrace(traceBuff);


        if (!found)
        {
          if (info.enabling == false)
          {
            result.appendValueToStringList("Route found (disabled)");
            found = true;
          } else
          {
            result.appendValueToStringList("Route not found");
            result.appendValueToStringList("");
          }
        } else
        {
          result.appendValueToStringList("Route found");
        }

        if (found)
        {
            char routeText[2048];
            char srcAddressText[64]; char dstAddressText[64];
            info.source.getText(srcAddressText, sizeof(srcAddressText));
            info.dest  .getText(dstAddressText, sizeof(dstAddressText));

            std::auto_ptr<char> encRouteId(getEncodedString(info.routeId.c_str()));
            std::auto_ptr<char> encSrcAddressText(getEncodedString(srcAddressText));
            std::auto_ptr<char> encDstAddressText(getEncodedString(dstAddressText));
            std::auto_ptr<char> encSmeSystemId(getEncodedString(info.smeSystemId.c_str()));
            std::auto_ptr<char> encForwardTo(getEncodedString(info.forwardTo.c_str()));
            std::auto_ptr<char> encSrcSmeSystemId(getEncodedString(info.srcSmeSystemId.c_str()));

            sprintf(routeText, "route id:%s;source address:%s;destination address:%s;"
                               "sme system id:%s;source sme system id:%s;"
                               "priority:%u;service id:%d;delivery mode:%u;forward to:%s;"
                               "billing:%s;archiving:%s;enabling:%s;suppress delivery reports:%s",
                    encRouteId.get(), encSrcAddressText.get(), encDstAddressText.get(),
                    encSmeSystemId.get(), encSrcSmeSystemId.get(),
                    info.priority, info.serviceId, info.deliveryMode, encForwardTo.get(),
                    (info.billing) ? "yes":"no" , (info.archived) ? "yes":"no",
                    (info.enabling) ? "yes":"no", (info.suppressDeliveryReports) ? "yes":"no");

            result.appendValueToStringList(routeText);
        }

        if(dealiasText.length())
        {
          result.appendValueToStringList(dealiasText.c_str());
          result.appendValueToStringList("");
        }

        for (int i=0; i<traceBuff.size(); i++)
            result.appendValueToStringList(traceBuff[i].c_str());

        return result;
    }
    catch (AdminException& aexc) {
        throw;
    }
    catch (ConfigException& cexc) {
        throw AdminException("Load routes config file failed. Cause: %s", cexc.what());
    }
    catch (std::exception& exc) {
        throw AdminException("Trace route failed. Cause: %s.", exc.what());
    }
    catch (...) {
        throw AdminException("Trace route failed. Cause is unknown.");
    }
}

void SmscComponent::applyAliases()
throw (AdminException)
{
  configs.aliasconfig->reload();
  smsc_app_runner->getApp()->reloadAliases(configs);
}

void SmscComponent::reReadConfigs()
throw (AdminException)
{
  configs.smemanconfig->reload();
  __trace__("reinit smsc config");
  configs.cfgman->reinit();
  configs.cfgman = & smsc::util::config::Manager::getInstance();
  smsc::resourcemanager::ResourceManager::reload(configs.cfgman->getString("core.locales"), configs.cfgman->getString("core.default_locale"));
  __trace__("config reinitialized");
  configs.aliasconfig->reload();
  configs.routesconfig->reload();
}

const char * const getProfileCodepageStr(int codepage)
{
  switch (codepage)
  {
  case smsc::profiler::ProfileCharsetOptions::Default: return "default";
  case smsc::profiler::ProfileCharsetOptions::Ucs2:    return "UCS2";
  case smsc::profiler::ProfileCharsetOptions::Latin1:  return "Latin1";
  case smsc::profiler::ProfileCharsetOptions::Ucs2AndLat:  return "UCS2&Latin1";
  default:                                             return "unknown";
  }
}

const char * const getProfileReportoptionsStr(int reportoptions)
{
  switch (reportoptions)
  {
  case smsc::profiler::ProfileReportOptions::ReportFull:  return "full";
  case smsc::profiler::ProfileReportOptions::ReportFinal: return "final";
  case smsc::profiler::ProfileReportOptions::ReportNone:  return "none";
  default:                                                return "unknown";
  }
}

const char * const getProfileMatchTypeStr(int matchType)
{
  switch (matchType)
  {
  case ProfilerMatchType::mtDefault:  return "default";
  case ProfilerMatchType::mtExact:    return "exact";
  case ProfilerMatchType::mtMask:     return "mask";
  default:                            return "unknown";
  }
}

Variant SmscComponent::profileLookupEx(const Arguments &args) throw (AdminException)
{
  try
  {
    Smsc * app;
    ProfilerInterface * profiler;
    if (isSmscRunning() && (app = smsc_app_runner->getApp()) && (profiler = app->getProfiler()))
    {
      Address address(args.Get("address").getStringValue());
      int matchType = ProfilerMatchType::mtExact;
      std::string matchAddress;
      Profile& profile(profiler->lookupEx(address, matchType, matchAddress));
      Variant result(StringListType);
      result.appendValueToStringList(getProfileCodepageStr(profile.codepage));
      result.appendValueToStringList(getProfileReportoptionsStr(profile.reportoptions));
      result.appendValueToStringList(profile.locale.c_str());
      result.appendValueToStringList(profile.hide ? "true":"false");
      result.appendValueToStringList(profile.hideModifiable ? "true":"false");
      result.appendValueToStringList(profile.divert.c_str());
      result.appendValueToStringList(profile.divertActive ? "true" : "false");
      result.appendValueToStringList(profile.divertModifiable ? "true" : "false");

      result.appendValueToStringList(getProfileMatchTypeStr(matchType));
      result.appendValueToStringList(matchAddress.c_str());
      return result;
    }
    else
      throw   AdminException("SMSC is not running");
  } catch (AdminException &e) {
    throw e;
  } catch (std::exception &e) {
    throw AdminException(e.what());
  } catch (...) {
    throw AdminException("Unknown exception");
  }
}
Variant SmscComponent::profileLookup(const Arguments &args)
throw (AdminException)
{
  try
  {
    const char * const addressString = args.Get("address").getStringValue();

    Smsc * app;
    ProfilerInterface * profiler;
    if (isSmscRunning() && (app = smsc_app_runner->getApp()) && (profiler = app->getProfiler()))
    {
      Address address(addressString);
      //app->AliasToAddress(alias, address);
      Profile& profile(profiler->lookup(address));
#ifdef SMSC_DEBUG
      char addr_str[smsc::sms::MAX_ADDRESS_VALUE_LENGTH+1];
      address.getValue(addr_str);
      logger.debug("lookup Profile:\n  %s: Address: \"%s\"[%u], numebring plan:%u, type of number:%u, ", addressString, addr_str, address.getLength(), address.getNumberingPlan(), address.getTypeOfNumber());
#endif
      Variant result(StringListType);
      result.appendValueToStringList(getProfileCodepageStr(profile.codepage));
      result.appendValueToStringList(getProfileReportoptionsStr(profile.reportoptions));
      result.appendValueToStringList(profile.locale.c_str());
      result.appendValueToStringList(profile.hide ? "true" : "false");
      result.appendValueToStringList(profile.hideModifiable ? "true" : "false");
      result.appendValueToStringList(profile.divert.c_str());
      result.appendValueToStringList(profile.divertActive ? "true" : "false");
      result.appendValueToStringList(profile.divertModifiable ? "true" : "false");
      return result;
    }
    else
      throw   AdminException("SMSC is not running");
  }
  catch (HashInvalidKeyException & e)
  {
    throw AdminException("Address not defined");
  }
}

void setProfileFromString(Profile &profile, const StringList& profileStrings)
throw (AdminException)
{
  StringList::const_iterator i = profileStrings.begin();
  const char* codepageStr       = *i++;
  const char* reportStr         = *i++;
  const char* localeStr         = *i++;
  const char* hideStr           = *i++;
  const char* hideModifiableStr = *i++;
  const char* divert            = *i++;
  const char* divertActive      = *i++;
  const char* divertModifiable  = *i;

  if (!codepageStr || !reportStr || !localeStr || !hideStr || !hideModifiableStr
    || !divert || !divertActive || !divertModifiable)
    throw AdminException("profile options misformatted");

  /*__trace2__("%s,%s,%s,%s,%s",
          codepageStr, reportStr, localeStr, hideStr, hideModifiableStr);*/

  if (strcmp("default", codepageStr) == 0)
  {
    profile.codepage = smsc::profiler::ProfileCharsetOptions::Default;
  }
  else if (strcmp("UCS2", codepageStr) == 0)
  {
    profile.codepage = smsc::profiler::ProfileCharsetOptions::Ucs2;
  }
  else if (strcmp("Latin1", codepageStr) == 0)
  {
    profile.codepage = smsc::profiler::ProfileCharsetOptions::Latin1;
  }
  else if (strcmp("UCS2&Latin1", codepageStr) == 0)
  {
    profile.codepage = smsc::profiler::ProfileCharsetOptions::Ucs2AndLat;
  }
  else
    throw   AdminException("Unknown charset");

  if (strcmp("full", reportStr) == 0)
  {
    profile.reportoptions = smsc::profiler::ProfileReportOptions::ReportFull;
  }
  else if (strcmp("final", reportStr) == 0)
  {
    profile.reportoptions = smsc::profiler::ProfileReportOptions::ReportFinal;
  }
  else if (strcmp("none", reportStr) == 0)
  {
    profile.reportoptions = smsc::profiler::ProfileReportOptions::ReportNone;
  }
  else
    throw   AdminException("Unknown report options");

  profile.locale = localeStr;

  profile.hide =       (strcmp("true", hideStr) == 0) ? 1:0;
  profile.hideModifiable = (strcmp("true", hideModifiableStr) == 0) ? true:false;

  profile.divert = divert;
  profile.divertActive =     (strcmp("true", divertActive) == 0) ? 1:0;
  profile.divertModifiable = (strcmp("true", divertModifiable) == 0) ? 1:0;
}

bool isMask(const Address & address)
{
  for (unsigned i=0; i<address.length; i++)
  {
    if (address.value[i] == '?')
      return true;
  }
  return false;
}

int SmscComponent::profileUpdate(const Arguments & args)
{
  try
  {
    const char * const addressString = args.Get("address").getStringValue();
    const StringList& profileStrings = args.Get("profile").getStringListValue();

    Smsc * app;
    ProfilerInterface * profiler;
    if (isSmscRunning() && (app = smsc_app_runner->getApp()) && (profiler = app->getProfiler()))
    {
      Address address(addressString);
      Profile profile;
      setProfileFromString(profile, profileStrings);
#ifdef SMSC_DEBUG
      char addr_str[smsc::sms::MAX_ADDRESS_VALUE_LENGTH+9];
      address.toString(addr_str, sizeof(addr_str)/sizeof(addr_str[0]));
      logger.debug("Update Profile:\n  %s: Address: \"%s\", codepage:%u, report options:%u, locale:%s", addressString, addr_str, profile.codepage, profile.reportoptions, profile.locale.c_str());
#endif
      if (isMask(address))
        return profiler->updatemask(address, profile);
      else
        return profiler->update(address, profile);
    }
    else
      throw   AdminException("SMSC is not running");
  }
  catch (HashInvalidKeyException &e)
  {
    throw AdminException("Address or profile params not defined");
  }
}

int SmscComponent::profileDelete(const Arguments & args)
{
  try
  {
    Smsc * app;
    ProfilerInterface * profiler;
    if (isSmscRunning() && (app = smsc_app_runner->getApp()) && (profiler = app->getProfiler()))
    {
      Address address(args.Get("address").getStringValue());
      profiler->remove(address);
    }
    else
      throw   AdminException("SMSC is not running");

    return 1;
  }
  catch (HashInvalidKeyException &e)
  {
    throw AdminException("Address or profile params not defined");
  }
}

void fillSmeInfo(SmeInfo & smeInfo, const Arguments & args)
{
  smeInfo.systemId          = args.Get("id").getStringValue();
  smeInfo.typeOfNumber      = args.Get("typeOfNumber").getLongValue();
  smeInfo.numberingPlan     = args.Get("numberingPlan").getLongValue();
  smeInfo.interfaceVersion  = args.Get("interfaceVersion").getLongValue();
  smeInfo.rangeOfAddress    = args.Get("addrRange").getStringValue();
  smeInfo.systemType        = args.Get("systemType").getStringValue();
  smeInfo.password          = args.Get("password").getStringValue();
  smeInfo.priority          = args.Get("priority").getLongValue();
  smeInfo.SME_N             = args.Get("smeN").getLongValue();
  smeInfo.disabled          = args.Get("disabled").getBooleanValue();
  smeInfo.wantAlias         = args.Get("wantAlias").getBooleanValue();
  smeInfo.forceDC           = args.Get("forceDC").getBooleanValue();
  smeInfo.receiptSchemeName = args.Get("receiptSchemeName").getStringValue();
  smeInfo.timeout           = args.Get("timeout").getLongValue();
  if (args.Exists("proclimit"))
    smeInfo.proclimit         = args.Get("proclimit").getLongValue();
  if (args.Exists("schedlimit"))
    smeInfo.schedlimit        = args.Get("schedlimit").getLongValue();
  const char * const mode = args.Get("mode").getStringValue();
  if (stricmp(mode,  "tx") == 0)
  {
    smeInfo.bindMode = smeTX;
  }
  else if (stricmp(mode, "rx") == 0)
  {
    smeInfo.bindMode = smeRX;
  }
  else if (stricmp(mode, "trx") == 0)
  {
    smeInfo.bindMode = smeTRX;
  }
}

SmeAdministrator * SmscComponent::getSmeAdmin()
{
  Smsc * app;
  SmeAdministrator * smeAdmin;
  if (isSmscRunning() && (app = smsc_app_runner->getApp()) && (smeAdmin = app->getSmeAdmin()))
    return smeAdmin;
  else
    throw   AdminException("SMSC is not running");
}

void SmscComponent::smeAdd(const Arguments & args)
{
  SmeInfo smeInfo;
  fillSmeInfo(smeInfo, args);
  getSmeAdmin()->addSme(smeInfo);
}

void SmscComponent::smeRemove(const Arguments & args)
{
  getSmeAdmin()->deleteSme(args.Get("id").getStringValue());
}

void SmscComponent::smeUpdate(const Arguments & args)
{
  SmeInfo smeInfo;
  fillSmeInfo(smeInfo, args);
  getSmeAdmin()->updateSmeInfo(smeInfo.systemId, smeInfo);
}

Variant SmscComponent::smeStatus(const Arguments & args)
{
  Variant result(StringListType);
  for (SmeIterator* i = getSmeAdmin()->iterator(); i != NULL;)
  {
    std::string status;
    status += i->getSmeInfo().systemId;
    status += ",";
    if (i->getSmeInfo().internal)
    {
      status += "internal";
    }
    else
    {
      if (!i->isSmeConnected())
      {
        status += "disconnected";
      }
      else
      {
        SmeProxy * smeProxy = i->getSmeProxy();
        try {
          std::string tmpStr;
          switch (smeProxy->getBindMode())
          {
            case smeTX:
              tmpStr += "tx,";
              break;
            case smeRX:
              tmpStr += "rx,";
              break;
            case smeTRX:
              tmpStr += "trx,";
              break;
            default:
              tmpStr += "unknown,";
          }
          char inIP[128], outIP[128];
          if (smeProxy->getPeers(inIP,outIP))
          {
            tmpStr += inIP;
            tmpStr += ",";
            tmpStr += outIP;
          }
          else
          {
            tmpStr += "unknown,unknown";
          }
          status += tmpStr;
        } catch (...) {
          status += "unknown,unknown,unknown";
        }
      }
    }
    result.appendValueToStringList(status.c_str());
    if (!i->next())
      break;
  }
  return result;
}

void SmscComponent::smeDisconnect(const Arguments & args)
{
  const StringList & ids = args.Get("ids").getStringListValue();
  for (StringList::const_iterator j = ids.begin(); j != ids.end(); j++)
  {
    std::string smeId(*j);
    for (SmeIterator* i = getSmeAdmin()->iterator(); i != NULL;)
    {
      if (i->getSmeProxy() != 0 && i->getSmeInfo().systemId == smeId)
      {
        logger.debug("Disconnecting sme \"%s\"", smeId.c_str());
        i->getSmeProxy()->disconnect();
      }
      if (!i->next())
        break;
    }
  }
}

Variant SmscComponent::logGetCategories(void)
{
  Variant result(service::StringListType);
  Logger::LogCats * cats = Logger::getCurrentCategories();
  for (Logger::LogCats::iterator i = cats->begin(); i != cats->end(); i++)
  {
    std::string tmp(i->first);
    tmp += ",";
    tmp += Logger::getLogLevelName(i->second);
    result.appendValueToStringList(tmp.c_str());
  }
  delete cats;
  return result;
}

void SmscComponent::setLogCat(const char * catStr)
{
  using namespace std;

  auto_ptr<char> str(new char[strlen(catStr) +1]);
  strcpy(str.get(), catStr);
  char * delim_pos = strrchr(str.get(), ',');
  if (delim_pos != NULL)
  {
    char * value = delim_pos+1;
    *delim_pos = 0;

  Logger::setCategoryLogLevel(str.get(), Logger::getLogLevel(value));
  }
  else
  {
    logger.error("misformatted logger category string: \"%s\"", catStr);
  }
}

void SmscComponent::logSetCategories(const Arguments & args)
{
  const StringList & cats = args.Get("categories").getStringListValue();
  for (StringList::const_iterator i = cats.begin(); i != cats.end(); i++)
  {
    setLogCat(*i);
  }
  if( smsc_app_runner->getApp()->getMapProxy() != 0 ) {
    dynamic_cast<MapProxy*>(smsc_app_runner->getApp()->getMapProxy())->checkLogging();
  }
}

Variant SmscComponent::applyLocaleResource()
throw (AdminException)
{
  if (isSmscRunning())
  {
    smsc::resourcemanager::ResourceManager::reload(configs.cfgman->getString("core.locales"), configs.cfgman->getString("core.default_locale"));
  }
  return Variant("");
}

}
}
}
