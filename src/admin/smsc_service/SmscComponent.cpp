#include "SmscComponent.h"
#include "profiler/profiler.hpp"

#include <system/smsc.hpp>

#include "router/route_types.h"

#include <util/config/Manager.h>
#include <util/config/route/RouteConfig.h>
#include <mscman/MscManager.h>
#include <resourcemanager/ResourceManager.hpp>
#include "acls/interfaces.h"
#include <algorithm>
#include "cluster/Interconnect.h"
#include "cluster/Commands.h"
#include "closedgroups/ClosedGroupsInterface.hpp"
#include "system/common/TimeZoneMan.hpp"
#ifdef SNMP
#include "system/snmp/SnmpCounter.hpp"
#endif
#ifdef USE_MAP
#include "system/mapio/FraudControl.hpp"
#include "system/mapio/MapLimits.hpp"
#include "system/mapio/MapProxy.h"
#endif

namespace smsc {

namespace util{
namespace config{
namespace smeman{
void fillFlag(uint32_t& flags,const std::string& val);
}
}
}

namespace admin {
namespace smsc_service {

using namespace smsc::system;
using namespace smsc::profiler;
using namespace smsc::router;
using namespace smsc::core::buffers;
using namespace smsc::util::config;
using namespace smsc::util::config::route;
using namespace smsc::acls;
using smsc::cluster::ApplyRoutesCommand;
using smsc::cluster::ApplyAliasesCommand;
using smsc::cluster::ApplyRescheduleCommand;
using smsc::cluster::ApplyLocaleResourceCommand;
using smsc::cluster::SmeAddCommand;
using smsc::cluster::SmeRemoveCommand;
using smsc::cluster::SmeUpdateCommand;

using smsc::mscman::MscManager;
using smsc::mscman::MscInfo;

using smsc::cluster::Interconnect;


SmscComponent::SmscComponent(SmscConfigs &all_configs, const char * node_)
: configs(all_configs),isStopping(false),
      logger(Logger::getInstance("smsc.admin.smsc_service.SmscComponent")),
    node(node_)
{
  using smsc::admin::service::StringType;
  using smsc::admin::service::StringListType;
  using smsc::admin::service::LongType;
  using smsc::admin::service::BooleanType;
  using smsc::admin::service::Parameter;
  using smsc::admin::service::Parameters;
  using smsc::admin::service::Method;
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
  //sme_params["interfaceVersion"  ] = Parameter("interfaceVersion"  , LongType);
  sme_params["systemType"        ] = Parameter("systemType"        , StringType);
  sme_params["password"          ] = Parameter("password"          , StringType);
  sme_params["addrRange"         ] = Parameter("addrRange"         , StringType);
  sme_params["smeN"              ] = Parameter("smeN"              , LongType);
  sme_params["wantAlias"         ] = Parameter("wantAlias"         , BooleanType);
  //sme_params["forceDC"           ] = Parameter("forceDC"           , BooleanType);
  sme_params["timeout"           ] = Parameter("timeout"           , LongType);
  sme_params["receiptSchemeName" ] = Parameter("receiptSchemeName" , StringType);
  sme_params["disabled"          ] = Parameter("disabled"          , BooleanType);
  sme_params["mode"              ] = Parameter("mode"              , StringType);
  sme_params["proclimit"         ] = Parameter("proclimit"         , LongType);
  sme_params["schedlimit"        ] = Parameter("schedlimit"        , LongType);
  sme_params["accessMask"        ] = Parameter("accessMask"        , LongType);
  sme_params["flags"             ] = Parameter("flags"             , StringType);

  Parameters sme_id_params;
  sme_id_params["id"] = Parameter("id", StringType);
  Parameters sme_ids_params;
  sme_ids_params["ids"] = Parameter("ids", StringListType);
  Parameters log_cats;
  log_cats["categories"] = Parameter("categories", StringListType);

  Parameters acl_id_params;
  acl_id_params["id"] = Parameter("id", LongType);
  Parameters acl_create_params;
  acl_create_params["name"] = Parameter("name", StringType);
  acl_create_params["description"] = Parameter("description", StringType);
  acl_create_params["cache_type"] = Parameter("cache_type", StringType);
  acl_create_params["addresses"] = Parameter("addresses", StringListType);
  Parameters acl_full_params;
  acl_full_params["id"] = Parameter("id", LongType);
  acl_full_params["name"] = Parameter("name", StringType);
  acl_full_params["description"] = Parameter("description", StringType);
  acl_full_params["addresses"] = Parameter("addresses", StringListType);
  Parameters acl_info_params;
  acl_info_params["id"] = Parameter("id", LongType);
  acl_info_params["name"] = Parameter("name", StringType);
  acl_info_params["description"] = Parameter("description", StringType);
  acl_info_params["cache_type"] = Parameter("cache_type", StringType);
  Parameters acl_lookup_addresses_params;
  acl_lookup_addresses_params["id"] = Parameter("id", LongType);
  acl_lookup_addresses_params["prefix"] = Parameter("prefix", StringType);
  Parameters acl_addresses_params;
  acl_addresses_params["id"] = Parameter("id", LongType);
  acl_addresses_params["addresses"] = Parameter("addresses", StringListType);

  // Interconnect manager params

  Parameters setRole_params;
  setRole_params["role"] = Parameter("role", StringType);

  /*---------------------loomox---------------*/



  Parameters prc_list_params;
  prc_list_params["address"] = Parameter("address", StringType);

  Parameters prc_add_params;
  prc_add_params["address"] = Parameter("address", StringType);
  prc_add_params["maxLists"] = Parameter("maxLists", LongType);
  prc_add_params["maxElements"] = Parameter("maxElements", LongType);

  Parameters prc_delete_params;
  prc_delete_params["address"] = Parameter("address", StringType);

  Parameters  prc_get_params;
  prc_get_params["address"] = Parameter("address", StringType);

  Parameters prc_alter_params;
  prc_alter_params["address"] = Parameter("address", StringType);
  prc_alter_params["maxLists"] = Parameter("maxLists", LongType);
  prc_alter_params["maxElements"] = Parameter("maxElements", LongType);
  prc_alter_params["altLists"] = Parameter("altLists", BooleanType);
  prc_alter_params["altElements"] = Parameter("altElements", BooleanType);

  Parameters mem_add_params;
  mem_add_params["dlname"] = Parameter("dlname", StringType);
  mem_add_params["address"] = Parameter("address", StringType);


  Parameters mem_delete_params;
  mem_delete_params["dlname"] = Parameter("dlname", StringType);
  mem_delete_params["address"] = Parameter("address", StringType);

  Parameters mem_get_params;
  mem_get_params["dlname"] = Parameter("dlname", StringType);

  Parameters sbm_add_params;
  sbm_add_params["dlname"] = Parameter("dlname", StringType);
  sbm_add_params["address"] = Parameter("address", StringType);

  Parameters sbm_delete_params;
  sbm_delete_params["dlname"] = Parameter("dlname", StringType);
  sbm_delete_params["address"] = Parameter("address", StringType);

  Parameters sbm_list_params;
  sbm_list_params["dlname"] = Parameter("dlname", StringType);

  Parameters dl_add_params;
  dl_add_params["dlname"] = Parameter("dlname", StringType);
  dl_add_params["owner"] = Parameter("owner", StringType);
  dl_add_params["maxElements"] = Parameter("maxElements", LongType);

  Parameters dl_delete_params;
  dl_delete_params["dlname"] = Parameter("dlname", StringType);

  Parameters dl_get_params;
  dl_get_params["dlname"] = Parameter("dlname", StringType);

  Parameters dl_list_params;
  dl_list_params["names"] = Parameter("names", StringListType);
  dl_list_params["owners"] = Parameter("owners", StringListType);

  Parameters dl_copyandrename_params;
  dl_copyandrename_params["dlname"]=Parameter("dlname",StringType);
  dl_copyandrename_params["newdlname"]=Parameter("newdlname",StringType);

  Parameters dl_alter_params;
  dl_alter_params["dlname"] = Parameter("dlname", StringType);
  dl_alter_params["maxElements"] = Parameter("maxElements", LongType);


  Parameters cgmAddGroupParams;
  cgmAddGroupParams["id"]=Parameter("id",LongType);
  cgmAddGroupParams["name"]=Parameter("name",StringType);

  Parameters cgmDeleteGroupParams;
  cgmDeleteGroupParams["id"]=Parameter("id",LongType);

  Parameters cgmAddAddrParams;
  cgmAddAddrParams["id"]=Parameter("id",LongType);
  cgmAddAddrParams["addr"]=Parameter("addr",StringType);

  Parameters cgmCheckParams;
  cgmCheckParams["id"]=Parameter("id",LongType);
  cgmCheckParams["addr"]=Parameter("addr",StringType);

  Parameters cgmDelAddrParams;
  cgmDelAddrParams["id"]=Parameter("id",LongType);
  cgmDelAddrParams["addr"]=Parameter("addr",StringType);

  Parameters cgmAddAbonentParams;
  cgmAddAbonentParams["id"]=Parameter("id",LongType);
  cgmAddAbonentParams["addr"]=Parameter("addr",StringType);

  Parameters cgmDelAbonentParams;
  cgmDelAbonentParams["id"]=Parameter("id",LongType);
  cgmDelAbonentParams["addr"]=Parameter("addr",StringType);

  Parameters cgmListAbonentsParams;
  cgmListAbonentsParams["id"]=Parameter("id",LongType);

  Parameters aliasAddParams;
  aliasAddParams["address"]=Parameter("address",StringType);
  aliasAddParams["alias"]=Parameter("alias",StringType);
  aliasAddParams["hide"]=Parameter("hide",LongType);

  Parameters aliasDelParams;
  aliasDelParams["alias"]=Parameter("alias",StringType);

#ifdef SMSEXTRA
  Parameters setSponsoredValuesParams;
  setSponsoredValuesParams["list"]=Parameter("list",StringListType);
  Method set_sponsored_values ((unsigned)setSponsoredValuesMethod,"set_sponsored_values",setSponsoredValuesParams,StringType);
#endif


  /**************************** method declarations *************************/
  Method apply_routes          ((unsigned)applyRoutesMethod,         "apply_routes",          empty_params, StringType);
  Method apply_aliases         ((unsigned)applyAliasesMethod,        "apply_aliases",         empty_params, StringType);
  Method apply_reschedule      ((unsigned)applyRescheduleMethod,     "apply_reschedule",      empty_params, StringType);
  Method apply_smsc_config     ((unsigned)applySmscConfigMethod,     "apply_smsc_config",     empty_params, StringType);
  Method apply_services        ((unsigned)applyServicesMethod,       "apply_services",        empty_params, StringType);
  Method apply_locale_resource ((unsigned)applyLocaleResourceMethod, "apply_locale_resources",empty_params, StringType);
  Method apply_timezones       ((unsigned)applyTimeZonesMethod,      "apply_timezones",       empty_params, StringType);
  Method apply_fraudcontrol    ((unsigned)applyFraudControlMethod,   "apply_fraud",           empty_params, StringType);
#ifdef USE_MAP
  Method apply_maplimits       ((unsigned)applyMapLimitsMethod,      "apply_maplimits",       empty_params, StringType);
#endif

#ifdef SNMP
  Method apply_snmp            ((unsigned)applySnmpMethod,           "apply_snmp",            empty_params, StringType);
#endif

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

  Method acl_list_names       ((unsigned)aclListNamesMethod,       "acl_list_names",        empty_params,                StringListType);
  Method acl_get              ((unsigned)aclGetMethod,             "acl_get",               acl_id_params,               StringListType);
  Method acl_remove           ((unsigned)aclRemoveMethod,          "acl_remove",            acl_id_params,               BooleanType);
  Method acl_create           ((unsigned)aclCreateMethod,          "acl_create",            acl_create_params,           LongType);
  Method acl_update_info      ((unsigned)aclUpdateInfoMethod,      "acl_update_info",       acl_info_params,             BooleanType);
  Method acl_lookup_addresses ((unsigned)aclLookupAddressesMethod, "acl_lookup_addresses",  acl_lookup_addresses_params, StringListType);
  Method acl_remove_addresses ((unsigned)aclRemoveAddressesMethod, "acl_remove_addresses",  acl_addresses_params,        BooleanType);
  Method acl_add_addresses    ((unsigned)aclAddAddressesMethod,    "acl_add_addresses",     acl_addresses_params,        BooleanType);

  /*-------------loomox-----------*/
  Method prc_list        ((unsigned)prcListPrincipalsMethod,   "prc_list",           prc_list_params,              StringListType);
  Method prc_add         ((unsigned)prcAddPrincipalMethod,     "prc_add",            prc_add_params,               StringType);
  Method prc_delete      ((unsigned)prcDeletePrincipalMethod,  "prc_delete",         prc_delete_params,            StringType);
  Method prc_get         ((unsigned)prcGetPrincipalMethod,     "prc_get",            prc_get_params,               StringListType);
  Method prc_alter       ((unsigned)prcAlterPrincipalMethod,   "prc_alter",          prc_alter_params,             StringType);

  Method mem_add         ((unsigned)memAddMemberMethod,        "mem_add",            mem_add_params,               StringType);
  Method mem_delete      ((unsigned)memDeleteMemberMethod,     "mem_delete",         mem_delete_params,            StringType);
  Method mem_get         ((unsigned)memGetMemberMethod,        "mem_get",            mem_get_params,               StringListType);

  Method sbm_add         ((unsigned)sbmAddSubmiterMethod,      "sbm_add",            sbm_add_params,               StringType);
  Method sbm_delete      ((unsigned)sbmDeleteSubmiterMethod,   "sbm_delete",         sbm_delete_params,            StringType);
  Method sbm_list        ((unsigned)sbmListSubmitersMethod,    "sbm_list",           sbm_list_params,              StringListType);

  Method dl_add          ((unsigned)dlAddMethod,               "dl_add",             dl_add_params,                StringType);
  Method dl_delete       ((unsigned)dlDeleteMethod,            "dl_delete",          dl_delete_params,             StringType);
  Method dl_get          ((unsigned)dlGetMethod,               "dl_get",             dl_get_params,                StringListType);
  Method dl_list         ((unsigned)dlListMethod,              "dl_list",            dl_list_params,               StringListType);
  Method dl_alter        ((unsigned)dlAlterMethod,             "dl_alter",           dl_alter_params,              StringType);
  Method dl_copy         ((unsigned)dlCopyMethod,              "dl_copy",            dl_copyandrename_params,      StringType);
  Method dl_rename       ((unsigned)dlRenameMethod,            "dl_rename",          dl_copyandrename_params,      StringType);


  // Interconnect manager methods

  Method set_role        ((unsigned)setRoleMethod,             "set_role",           setRole_params,               StringType);
  Method get_role        ((unsigned)getRoleMethod,             "get_role",           empty_params,                 StringType);


  Method cgm_addgroup((unsigned)cgmAddGroupMethod,"cgm_addgroup",cgmAddGroupParams,StringType);
  Method cgm_deletegroup((unsigned)cgmDeleteGroupMethod,"cgm_deletegroup",cgmDeleteGroupParams,StringType);
  Method cgm_addaddr((unsigned)cgmAddAddrMethod,"cgm_addaddr",cgmAddAddrParams,StringType);
  Method cgm_check((unsigned)cgmCheckMethod,"cgm_check",cgmCheckParams,StringType);
  Method cgm_deladdr((unsigned)cgmDelAddrMethod,"cgm_deladdr",cgmDelAddrParams,StringType);
  Method cgm_addabonent((unsigned)cgmAddAbonentMethod,"cgm_addabonent",cgmAddAbonentParams,StringType);
  Method cgm_delabonent((unsigned)cgmDelAbonentMethod,"cgm_delabonent",cgmDelAbonentParams,StringType);
  Method cgm_listabonents((unsigned)cgmListAbonentsMethod,"cgm_listabonents",cgmListAbonentsParams,StringListType);

  Method alias_add((unsigned)aliasAddMethod,"alias_add",aliasAddParams,StringType);
  Method alias_del((unsigned)aliasDelMethod,"alias_del",aliasDelParams,StringType);

  /***************************** method assigns *****************************/
#ifdef SMSEXTRA
  methods[set_sponsored_values .getName()] = set_sponsored_values;
#endif

  methods[apply_routes         .getName()] = apply_routes;
  methods[apply_aliases        .getName()] = apply_aliases;
  methods[apply_reschedule     .getName()] = apply_reschedule;
  methods[apply_smsc_config    .getName()] = apply_smsc_config;
  methods[apply_services       .getName()] = apply_services;
  methods[apply_locale_resource.getName()] = apply_locale_resource;
  methods[apply_timezones      .getName()] = apply_timezones;
  methods[apply_fraudcontrol   .getName()] = apply_fraudcontrol;
#ifdef USE_MAP
  methods[apply_maplimits      .getName()] = apply_maplimits;
#endif

#ifdef SNMP
  methods[apply_snmp           .getName()] = apply_snmp;
#endif

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

  methods[acl_list_names.getName()] = acl_list_names;
  methods[acl_get.getName()] = acl_get;
  methods[acl_remove.getName()] = acl_remove;
  methods[acl_create.getName()] = acl_create;
  methods[acl_update_info.getName()] = acl_update_info;
  methods[acl_lookup_addresses.getName()] = acl_lookup_addresses;
  methods[acl_remove_addresses.getName()] = acl_remove_addresses;
  methods[acl_add_addresses.getName()] = acl_add_addresses;

  // Interconnect manager methods

  methods[set_role.getName()] = set_role;
  methods[get_role.getName()] = get_role;

  /*--------loomox------------*/

  methods[prc_list.getName()] = prc_list;
  methods[prc_add.getName()] = prc_add;
  methods[prc_delete.getName()] = prc_delete;
  methods[prc_get.getName()] = prc_get;
  methods[prc_alter.getName()] = prc_alter;

  methods[mem_add.getName()] = mem_add;
  methods[mem_delete.getName()] = mem_delete;
  methods[mem_get.getName()] = mem_get;

  methods[sbm_add.getName()] = sbm_add;
  methods[sbm_delete.getName()] = sbm_delete;
  methods[sbm_list.getName()] = sbm_list;

  methods[dl_add.getName()] = dl_add;
  methods[dl_delete.getName()] = dl_delete;
  methods[dl_get.getName()] = dl_get;
  methods[dl_list.getName()] = dl_list;
  methods[dl_alter.getName()] = dl_alter;
  methods[dl_copy.getName()] = dl_copy;
  methods[dl_rename.getName()] = dl_rename;

  methods[cgm_addgroup.getName()] = cgm_addgroup;
  methods[cgm_deletegroup.getName()] = cgm_deletegroup;
  methods[cgm_addaddr.getName()] = cgm_addaddr;
  methods[cgm_check.getName()] = cgm_check;
  methods[cgm_deladdr.getName()] = cgm_deladdr;
  methods[cgm_addabonent.getName()] = cgm_addabonent;
  methods[cgm_delabonent.getName()] = cgm_delabonent;

  methods[alias_add.getName()] = alias_add;
  methods[alias_del.getName()] = alias_del;

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
    smsc_log_debug(logger, "call \"%s\"", method.getName());
    switch (method.getId())
    {
      case applyRoutesMethod:
        smsc_log_debug(logger, "applying routes...");
        applyRoutes();
        smsc_log_debug(logger, "routes applied");
        return Variant("");
      case applyAliasesMethod:
        smsc_log_debug(logger, "applying aliases...");
        applyAliases();
        smsc_log_debug(logger, "aliases applied");
        return Variant("");
      case applyRescheduleMethod:
        smsc_log_debug(logger, "applying reschedule...");
        applyReschedule();
        smsc_log_debug(logger, "reschedule applied");
        return Variant("");
#ifdef SNMP
      case applySnmpMethod:
        smsc_log_debug(logger, "applying snmp...");
        applySnmp();
        smsc_log_debug(logger, "snmp applied");
        return Variant("");
#endif
#ifdef SMSEXTRA
      case setSponsoredValuesMethod:
        setSponsoredValue(args);
        return Variant("");
#endif
      case profileLookupMethod:
        smsc_log_debug(logger, "lookup profile...");
        return profileLookup(args);
      case profileLookupExMethod:
        smsc_log_debug(logger, "lookupEx profile...");
        return profileLookupEx(args);
      case profileUpdateMethod:
        smsc_log_debug(logger, "update profile...");
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
      case applyTimeZonesMethod:
        applyTimeZones();
        return Variant("");
#ifdef USE_MAP
      case applyFraudControlMethod:
        applyFraudControl();
        return Variant("");
      case applyMapLimitsMethod:
        applyMapLimits();
        return Variant("");
#endif
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

      case aclListNamesMethod:
        return aclListNames(args);
      case aclGetMethod:
        return aclGet(args);
      case aclRemoveMethod:
        return aclRemove(args);
      case aclCreateMethod:
        return aclCreate(args);
      case aclUpdateInfoMethod:
        return aclUpdateInfo(args);
      case aclLookupAddressesMethod:
        return aclLookupAddresses(args);
      case aclRemoveAddressesMethod:
        return aclRemoveAddresses(args);
      case aclAddAddressesMethod:
        return aclAddAddresses(args);

      case getRoleMethod:
        return getRole();
      case setRoleMethod:
        return setRole(args);


/*------loomox-----------*/
      case prcListPrincipalsMethod:
        return dlListPrincipals(args);
      case prcAddPrincipalMethod:
        return dlAddPrincipal(args);
      case prcDeletePrincipalMethod:
        return dlDelPrincipal(args);
      case prcGetPrincipalMethod:
        return dlGetPrincipal(args);
      case prcAlterPrincipalMethod:
        return dlAlterPrincipal(args);


      case memAddMemberMethod:
        return dlAddMember(args);
      case memDeleteMemberMethod:
        return dlDeleteMember(args);
      case memGetMemberMethod:
        return dlGetMembers(args);

      case sbmAddSubmiterMethod:
        return dlAddSubmitter(args);
      case sbmDeleteSubmiterMethod:
        return dlDeleteSubmitter(args);
      case sbmListSubmitersMethod:
        return dlListSubmitters(args);

      case dlAddMethod:
        return dlAddList(args);
      case dlDeleteMethod:
        return dlDeleteList(args);
      case dlGetMethod:
        return dlGetList(args);
      case dlListMethod:
        return dlListLists(args);
      case dlAlterMethod:
        return dlAlterList(args);
      case dlCopyMethod:
        return dlCopyList(args);
      case dlRenameMethod:
        return dlRenameList(args);

      case cgmAddGroupMethod:
        return cgmAddGroup(args);
      case cgmDeleteGroupMethod:
        return cgmDeleteGroup(args);
      case cgmAddAddrMethod:
        return cgmAddAddr(args);
      case cgmCheckMethod:
        return cgmCheck(args);
      case cgmDelAddrMethod:
        return cgmDelAddr(args);
      case cgmAddAbonentMethod:
        return cgmAddAbonent(args);
      case cgmDelAbonentMethod:
        return cgmDelAbonent(args);
      case cgmListAbonentsMethod:
        return cgmListAbonents(args);
      case aliasAddMethod:
        return addAlias(args);
      case aliasDelMethod:
        return delAlias(args);

      default:
        smsc_log_debug(logger, "unknown method \"%s\" [%u]", method.getName(), method.getId());
        throw AdminException("Unknown method \"%s\"", method.getName());
    }
    smsc_log_error(logger, "call \"%s\"[%u] done. Unreacheable code reached.", method.getName(), method.getId());
    return Variant("");
  }
  catch (AdminException &e)
  {
    throw;
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
  smsc_log_info(logger, "applying new services...");
  reloadConfigsAndRestart();
  smsc_log_info(logger, "new services applied.");
}

void SmscComponent::applySmscConfig()
throw (AdminException)
{
  smsc_log_info(logger, "applying new configs...");
  reloadConfigsAndRestart();
  smsc_log_info(logger, "new config applied.");
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
    smsc_log_error(logger, "Couldn't apply new config: %s", e.what());
    return;
  }
  catch (...)
  {
    smsc_log_error(logger, "Couldn't apply new config: unknown exception");
    return;
  }
}

std::string SmscComponent::flushStatistics(const Arguments &args)
throw (AdminException)
{
  smsc_log_debug(logger, "flushStatistics");
  try
  {
    smsc_app_runner->getApp()->flushStatistics();
  }
  catch (std::exception &e)
  {
    smsc_log_error(logger, "Exception on flush statistics: %s", e.what());
    throw AdminException("Exception on flush statistics");
  }
  catch (...)
  {
    smsc_log_error(logger, "Unknown exception on flush statistics");
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
    size_t length = ptr-str;
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
    smsc_log_info(logger, "message %s[%s][%s] canceled", sid.c_str(), ssrc.c_str(), sdst.c_str());
  }
  catch (...)
  {
    smsc_log_error(logger, "Couldn't cancel message %s[%s][%s]", sid.c_str(), ssrc.c_str(), sdst.c_str());
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
    smsc_log_info(logger, "processCancelMessages: [%s], [%s], [%s]", idsStr, sourcesStr, destinationsStr);

    StrArray ids = parseStringToStringList(idsStr);
    StrArray sources = parseStringToStringList(sourcesStr);
    StrArray destinations = parseStringToStringList(destinationsStr);
    if ((ids.Count() != sources.Count())
       || (ids.Count() != destinations.Count())
       || (destinations.Count() != sources.Count()))
    {
      smsc_log_error(logger, "processCancelMessages failed: incorrect params");
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
    smsc_log_error(logger, "processCancelMessages failed: not all parameters defined");
    throw AdminException("not all parameters defined");
  }
  catch (...)
  {
    smsc_log_error(logger, "processCancelMessages failed: unknown reason");
    throw AdminException("unknown exception");
  }
}

void SmscComponent::runSmsc()
throw (AdminException)
{
  {
    smsc::core::synchronization::MutexGuard guard(mutex);
    if (smsc_app_runner.get() == 0)
    {
      try
      {
        smsc_app_runner.reset(new SmscAppRunner(configs, node.c_str()));
        smsc::resourcemanager::ResourceManager::reload(configs.cfgman->getString("core.locales"), configs.cfgman->getString("core.default_locale"));
        smsc_app_runner->Start();
      }
      catch (smsc::util::Exception &e)
      {
        smsc_log_error(logger, "Exception on starting SMSC: \"%s\"", e.what());
        throw AdminException("Exception on starting SMSC: \"%s\"", e.what());
      }
      catch (std::exception &e)
      {
        smsc_log_error(logger, "Exception on starting SMSC: \"%s\"", e.what());
        throw AdminException("Exception on starting SMSC: \"%s\"", e.what());
      }
      catch (...)
      {
        smsc_log_error(logger, "Unknown exception on starting SMSC");
        throw AdminException("Unknown exception on starting SMSC");
      }
    }
    else
      throw   AdminException("SMSC Application started already (or not sucessfully stopped)");
  }
  smsc_app_runner->WaitFor();
}

void SmscComponent::sigStopSmsc()
{
  smsc::core::synchronization::MutexGuard guard(mutex);
  if (smsc_app_runner.get() != 0)
  {
    smsc_app_runner->stop();
  }
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
      int rv=smsc_app_runner->WaitFor();
      if(rv!=0)
      {
        smsc_log_error(logger, "thr_join returned: %d", rv);
        return;
      }
      //smsc_app_runner.reset(0);
      isStopping = false;
    }
    catch (smsc::util::Exception &e)
    {
      isStopping = false;
      smsc_log_error(logger, "Exception on starting SMSC: \"%s\"", e.what());
      throw AdminException("Exception on stopping SMSC: \"%s\"", e.what());
    }
    catch (std::exception &e)
    {
      isStopping = false;
      smsc_log_error(logger, "Exception on stopping SMSC: \"%s\"", e.what());
      throw AdminException("Exception on starting SMSC: \"%s\"", e.what());
    }
    catch (...)
    {
      isStopping = false;
      smsc_log_error(logger, "Unknown exception on stopping SMSC");
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
      smsc_log_error(logger, "Exception on aborting SMSC: \"%s\"", e.what());
      throw AdminException("Exception on aborting SMSC: \"%s\"", e.what());
    }
    catch (std::exception &e)
    {
      smsc_log_error(logger, "Exception on aborting SMSC: \"%s\"", e.what());
    }
    catch (...)
    {
      smsc_log_error(logger, "Unknown exception on aborting SMSC");
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
      smsc_log_error(logger, "Exception on starting SMSC: \"%s\"", e.what());
      throw AdminException("Exception on dumping SMSC: \"%s\"", e.what());
    }
    catch (std::exception &e)
    {
      smsc_log_error(logger, "Exception on dumping SMSC: \"%s\"", e.what());
    }
    catch (...)
    {
      smsc_log_error(logger, "Unknown exception on dumping SMSC");
    }
  }
}

void SmscComponent::applyRoutes()
throw (AdminException)
{
  configs.routesconfig->reload();
  configs.smemanconfig->reload();
  smsc_app_runner->getApp()->reloadRoutes();

  if(smsc_app_runner->getApp()->isHs()){
      Interconnect * iconn = Interconnect::getInstance();
      if(!iconn){
          smsc_log_warn(logger, "InterconnectManager undefined, can't send applyRoutes command");
          return;
      }
      ApplyRoutesCommand * cmd = new ApplyRoutesCommand();
      iconn->sendCommand(cmd);
  }

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

        bool found = false;

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
          dealiasText="Address ";
          dealiasText+=Address(dstAddr).toString().c_str();
          dealiasText+=" was dealiased to ";
          dealiasText+=dealiased.toString().c_str();
          dealiased.toString(addrBuf,sizeof(addrBuf));
          dstAddr=addrBuf;
        }

        SmeIndex index = -1;
        smsc::router::RouteResult rr;

        if (srcSysId)
        {
            index = smsc_app_runner->getApp()->getSmeIndex(srcSysId);
            if (index == -1)
                throw AdminException("Trace route failed. Sme for system id '%s' not found", srcSysId);

        }
        found = smsc_app_runner->getApp()->getTestRouterInstance()->
            lookup(index, Address(srcAddr), Address(dstAddr), rr);

        vector<std::string> traceBuff;
        smsc_app_runner->getApp()->getTestRouterInstance()->getTrace(traceBuff);


        if (!found)
        {
          if (rr.found == false)
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
            //char srcAddressText[64]; char dstAddressText[64];


            //info.source.getText(srcAddressText, sizeof(srcAddressText));
            //info.dest  .getText(dstAddressText, sizeof(dstAddressText));

            std::auto_ptr<char> encRouteId(getEncodedString(rr.info.routeId.c_str()));
            std::auto_ptr<char> encSrcAddressText(getEncodedString(rr.rp.source.toString().c_str()));
            std::auto_ptr<char> encDstAddressText(getEncodedString(rr.rp.dest.toString().c_str()));
            std::auto_ptr<char> encSmeSystemId(getEncodedString(rr.info.smeSystemId.c_str()));
            std::auto_ptr<char> encForwardTo(getEncodedString(rr.info.forwardTo.c_str()));
            std::auto_ptr<char> encSrcSmeSystemId(getEncodedString(rr.info.srcSmeSystemId.c_str()));

            sprintf(routeText, "route id:%s;source address:%s;destination address:%s;"
                               "sme system id:%s;source sme system id:%s;"
                               "priority:%u;service id:%d;delivery mode:%u;forward to:%s;"
                               "billing:%s;archiving:%s;enabling:%s;suppress delivery reports:%s",
                    encRouteId.get(), encSrcAddressText.get(), encDstAddressText.get(),
                    encSmeSystemId.get(), encSrcSmeSystemId.get(),
                    rr.info.priority, rr.info.serviceId, rr.info.deliveryMode, encForwardTo.get(),
                    (rr.info.billing) ? "yes":"no" , (rr.info.archived) ? "yes":"no",
                    (rr.info.enabling) ? "yes":"no", (rr.info.suppressDeliveryReports) ? "yes":"no");

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
  /*
  //configs.aliasconfig->reload();
  //smsc_app_runner->getApp()->reloadAliases(configs);

  if(smsc_app_runner->getApp()->isHs()){
      Interconnect * iconn = Interconnect::getInstance();
      if(!iconn){
          smsc_log_warn(logger, "InterconnectManager undefined, can't send applyAliases command");
          return;
      }
      ApplyAliasesCommand * cmd = new ApplyAliasesCommand();
      iconn->sendCommand(cmd);
  }
  */
}

void SmscComponent::applyReschedule()
throw (AdminException)
{
  try{
    smsc_app_runner->getApp()->reloadReschedule();
  }catch(std::exception& e)
  {
    throw AdminException("%s",e.what());
  }

  if(smsc_app_runner->getApp()->isHs()){
      Interconnect * iconn = Interconnect::getInstance();
      if(!iconn){
          smsc_log_warn(logger, "InterconnectManager undefined, can't send applyReschedule command");
          return;
      }
      ApplyRescheduleCommand * cmd = new ApplyRescheduleCommand();
      iconn->sendCommand(cmd);
  }
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
  //configs.aliasconfig->reload();
  configs.routesconfig->reload();
}

const char * const getProfileCodepageStr(int codepage)
{
  switch (codepage & 0x7F)
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

const char * const getProfileHideOptionString(int ho)
{
  switch(ho)
  {
    case HideOption::hoDisabled:   return "false";
    case HideOption::hoEnabled:    return "true";
    case HideOption::hoSubstitute: return "substitute";
    default:                       return "unknown";
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
      result.appendValueToStringList(getProfileHideOptionString(profile.hide));
      result.appendValueToStringList(profile.hideModifiable ? "true":"false");
      result.appendValueToStringList(profile.divert.c_str());
      char divertActive[6];
      divertActive[0] = profile.divertActive         ? 'Y' : 'N';
      divertActive[1] = profile.divertActiveAbsent   ? 'Y' : 'N';
      divertActive[2] = profile.divertActiveBlocked  ? 'Y' : 'N';
      divertActive[3] = profile.divertActiveBarred   ? 'Y' : 'N';
      divertActive[4] = profile.divertActiveCapacity ? 'Y' : 'N';
      divertActive[5] = 0;
      result.appendValueToStringList(divertActive);
      result.appendValueToStringList(profile.divertModifiable ? "true" : "false");
      result.appendValueToStringList((profile.codepage & 0x80) != 0 ? "true" : "false");
      result.appendValueToStringList(profile.udhconcat ? "true" : "false");
      result.appendValueToStringList(profile.translit ? "true" : "false");

      char numBuf[32];
      sprintf(numBuf,"%u",profile.closedGroupId);
      result.appendValueToStringList(numBuf);
      sprintf(numBuf,"%u",profile.accessMaskIn);
      result.appendValueToStringList(numBuf);
      sprintf(numBuf,"%u",profile.accessMaskOut);
      result.appendValueToStringList(numBuf);
#ifdef SMSEXTRA
      sprintf(numBuf,"%u",profile.subscription);
      result.appendValueToStringList(numBuf);
      sprintf(numBuf,"%u",profile.sponsored);
      result.appendValueToStringList(numBuf);
      result.appendValueToStringList(profile.nick.c_str());
#endif

      result.appendValueToStringList(getProfileMatchTypeStr(matchType));
      result.appendValueToStringList(matchAddress.c_str());


      return result;
    }
    else
      throw   AdminException("SMSC is not running");
  } catch (AdminException &e) {
    throw;
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
      smsc_log_debug(logger, "lookup Profile:\n  %s: Address: \"%s\"[%u], numebring plan:%u, type of number:%u, ", addressString, addr_str, address.getLength(), address.getNumberingPlan(), address.getTypeOfNumber());
#endif
      Variant result(StringListType);
      result.appendValueToStringList(getProfileCodepageStr(profile.codepage));
      result.appendValueToStringList(getProfileReportoptionsStr(profile.reportoptions));
      result.appendValueToStringList(profile.locale.c_str());
      result.appendValueToStringList(getProfileHideOptionString(profile.hide));
      result.appendValueToStringList(profile.hideModifiable ? "true" : "false");
      result.appendValueToStringList(profile.divert.c_str());
      char divertActive[6];
      divertActive[0] = profile.divertActive         ? 'Y' : 'N';
      divertActive[1] = profile.divertActiveAbsent   ? 'Y' : 'N';
      divertActive[2] = profile.divertActiveBlocked  ? 'Y' : 'N';
      divertActive[3] = profile.divertActiveBarred   ? 'Y' : 'N';
      divertActive[4] = profile.divertActiveCapacity ? 'Y' : 'N';
      divertActive[5] = 0;
      result.appendValueToStringList(divertActive);
      result.appendValueToStringList(profile.divertModifiable ? "true" : "false");
      result.appendValueToStringList((profile.codepage & 0x80) != 0 ? "true" : "false");
      result.appendValueToStringList(profile.udhconcat ? "true" : "false");
      result.appendValueToStringList(profile.translit ? "true" : "false");
      char numBuf[32];
      sprintf(numBuf,"%u",profile.closedGroupId);
      result.appendValueToStringList(numBuf);
      sprintf(numBuf,"%u",profile.accessMaskIn);
      result.appendValueToStringList(numBuf);
      sprintf(numBuf,"%u",profile.accessMaskOut);
      result.appendValueToStringList(numBuf);
#ifdef SMSEXTRA
      sprintf(numBuf,"%u",profile.subscription);
      result.appendValueToStringList(numBuf);
      sprintf(numBuf,"%u",profile.sponsored);
      result.appendValueToStringList(numBuf);
      result.appendValueToStringList(profile.nick.c_str());
#endif
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
  const char* divertModifiable  = *i++;
  const char* ussd7bit          = *i++;
  const char* udhConcat         = *i++;
  const char* translit          = *i++;
  const char* closedGroupId     = *i++;
  const char* accessMaskIn      = *i++;
#ifndef SMSEXTRA
  const char* accessMaskOut     = *i;
#else
  const char* accessMaskOut     = *i++;
  const char* subscription      = *i++;
  const char* sponsored         = *i++;
  const char* nick              = *i;
#endif

  if (!codepageStr || !reportStr || !localeStr || !hideStr || !hideModifiableStr
    || !divert || !divertActive || !divertModifiable || !ussd7bit)
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

  if (strcmp("false", hideStr) == 0)
    profile.hide = smsc::profiler::HideOption::hoDisabled;
  else if(strcmp("true", hideStr) == 0)
    profile.hide = smsc::profiler::HideOption::hoEnabled;
  else if(strcmp("substitute", hideStr) == 0)
    profile.hide = smsc::profiler::HideOption::hoSubstitute;

  profile.hideModifiable = (strcmp("true", hideModifiableStr) == 0) ? true:false;

  profile.divert = divert;

  if (strlen(divertActive) != 5)
    throw   AdminException("Unknown divert active options");
  profile.divertActive         =     ((divertActive[0] == 'Y') || (divertActive[0] == 'y')) ? true : false;
  profile.divertActiveAbsent   =     ((divertActive[1] == 'Y') || (divertActive[1] == 'y')) ? true : false;
  profile.divertActiveBlocked  =     ((divertActive[2] == 'Y') || (divertActive[2] == 'y')) ? true : false;
  profile.divertActiveBarred   =     ((divertActive[3] == 'Y') || (divertActive[3] == 'y')) ? true : false;
  profile.divertActiveCapacity =     ((divertActive[4] == 'Y') || (divertActive[4] == 'y')) ? true : false;

  profile.divertModifiable = (strcmp("true", divertModifiable) == 0) ? 1:0;
  if (strcmp("true", ussd7bit) == 0)
    profile.codepage |= 0x80;

  profile.udhconcat = (strcmp("true", udhConcat) == 0) ? true : false;
  profile.translit = (strcmp("true", translit) == 0) ? true : false;

  sscanf(closedGroupId,"%u",&profile.closedGroupId);
  sscanf(accessMaskIn,"%u",&profile.accessMaskIn);
  sscanf(accessMaskOut,"%u",&profile.accessMaskOut);
#ifdef SMSEXTRA
  sscanf(subscription,"%u",&profile.subscription);
  uint32_t sponsoredValue;
  sscanf(sponsored,"%u",&sponsoredValue);
  profile.sponsored=(uint8_t)sponsoredValue;
  profile.nick=nick;
#endif

}

static bool isMask(const Address & address)
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
      smsc_log_debug(logger, "Update Profile:  %s: Address: \"%s\", codepage:%u, report options:%u, locale:%s, hide:%u, hideModif:%u", addressString, addr_str, profile.codepage, profile.reportoptions, profile.locale.c_str(), profile.hide, profile.hideModifiable);
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
  smeInfo.typeOfNumber      = (uint8_t)args.Get("typeOfNumber").getLongValue();
  smeInfo.numberingPlan     = (uint8_t)args.Get("numberingPlan").getLongValue();
  //smeInfo.interfaceVersion  = (uint8_t)args.Get("interfaceVersion").getLongValue();
  smeInfo.rangeOfAddress    = args.Get("addrRange").getStringValue();
  smeInfo.systemType        = args.Get("systemType").getStringValue();
  smeInfo.password          = args.Get("password").getStringValue();
  smeInfo.priority          = (SmeProxyPriority)args.Get("priority").getLongValue();
  smeInfo.SME_N             = (SmeNType)args.Get("smeN").getLongValue();
  smeInfo.disabled          = args.Get("disabled").getBooleanValue();
  smeInfo.wantAlias         = args.Get("wantAlias").getBooleanValue();
  //smeInfo.forceDC           = args.Get("forceDC").getBooleanValue();
  smeInfo.receiptSchemeName = args.Get("receiptSchemeName").getStringValue();
  smeInfo.timeout           = (uint32_t)args.Get("timeout").getLongValue();
  smeInfo.flags=0;
  if(args.Exists("flags"))
  {
    std::string v=args.Get("flags").getStringValue();
    std::string::size_type oldPos=0,commaPos=0;
    while((commaPos=v.find(',',oldPos))!=std::string::npos)
    {
      smsc::util::config::smeman::fillFlag(smeInfo.flags,v.substr(oldPos,commaPos-oldPos));
      oldPos=commaPos+1;
    }
    smsc::util::config::smeman::fillFlag(smeInfo.flags,v.substr(oldPos));
  }
  if(args.Exists("accessMask"))
  {
    smeInfo.accessMask=(uint32_t)args.Get("accessMask").getLongValue();
  }else
  {
    smeInfo.accessMask=1;
  }
  if (args.Exists("proclimit"))
    smeInfo.proclimit         = (uint32_t)args.Get("proclimit").getLongValue();
  if (args.Exists("schedlimit"))
    smeInfo.schedlimit        = (uint32_t)args.Get("schedlimit").getLongValue();
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

  if(smsc_app_runner->getApp()->isHs()){
      Interconnect * iconn = Interconnect::getInstance();
      if(!iconn){
          smsc_log_warn(logger, "InterconnectManager undefined, can't send SmeAdd command");
          return;
      }
      SmeAddCommand * cmd = new SmeAddCommand(smeInfo);
      iconn->sendCommand(cmd);
  }
}

void SmscComponent::smeRemove(const Arguments & args)
{
  getSmeAdmin()->deleteSme(args.Get("id").getStringValue());

  if(smsc_app_runner->getApp()->isHs()){
      Interconnect * iconn = Interconnect::getInstance();
      if(!iconn){
          smsc_log_warn(logger, "InterconnectManager undefined, can't send SmeRemove command");
          return;
      }
      SmeRemoveCommand * cmd = new SmeRemoveCommand(args.Get("id").getStringValue());
      iconn->sendCommand(cmd);
  }
}

void SmscComponent::smeUpdate(const Arguments & args)
{
  SmeInfo smeInfo;
  fillSmeInfo(smeInfo, args);
  getSmeAdmin()->updateSmeInfo(smeInfo.systemId, smeInfo);
  Smsc * app;
  if (isSmscRunning() && (app = smsc_app_runner->getApp())) {

  }

  if(smsc_app_runner->getApp()->isHs()){
      Interconnect * iconn = Interconnect::getInstance();
      if(!iconn){
          smsc_log_warn(logger, "InterconnectManager undefined, can't send SmeUpdate command");
          return;
      }
      SmeUpdateCommand * cmd = new SmeUpdateCommand(smeInfo);
      iconn->sendCommand(cmd);
  }
}

Variant SmscComponent::smeStatus(const Arguments & args)
{
  Variant result(StringListType);
  for (std::auto_ptr<SmeIterator> i(getSmeAdmin()->iterator()); i.get() != NULL;)
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
    for (std::auto_ptr<SmeIterator> i (getSmeAdmin()->iterator()); i.get() != NULL;)
    {
      if (i->getSmeProxy() != 0 && i->getSmeInfo().systemId == smeId)
      {
        smsc_log_debug(logger, "Disconnecting sme \"%s\"", smeId.c_str());
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
  std::auto_ptr<const Logger::LogLevels> cats(Logger::getLogLevels());
  char * k;
  Logger::LogLevel level;
  for (Logger::LogLevels::Iterator i = cats->getIterator(); i.Next(k, level); )
  {
    std::string tmp(k);
    tmp += ",";
    tmp += Logger::getLogLevel(level);
    result.appendValueToStringList(tmp.c_str());
  }
  return result;
}

void SmscComponent::logSetCategories(const Arguments & args)
{
  const StringList & cats = args.Get("categories").getStringListValue();
  Logger::LogLevels levels;
  for (StringList::const_iterator i = cats.begin(); i != cats.end(); i++)
  {
    std::auto_ptr<char> str(cStringCopy(*i));
    char * delim_pos = strrchr(str.get(), ',');
    if (delim_pos != NULL)
    {
      char * value = delim_pos+1;
      *delim_pos = 0;
      levels[str.get()] = Logger::getLogLevel(value);
    }
    else
    {
      smsc_log_error(logger, "misformatted logger category string: \"%s\"", str.get());
    }
  }

  Logger::setLogLevels(levels);

  /*
  if( smsc_app_runner->getApp()->getMapProxy() != 0 ) {
    dynamic_cast<smsc::system::mapio::MapProxy*>(smsc_app_runner->getApp()->getMapProxy())->checkLogging();
  }
  */
}

Variant SmscComponent::applyLocaleResource()
throw (AdminException)
{
  if (isSmscRunning())
  {
    smsc::resourcemanager::ResourceManager::reload(configs.cfgman->getString("core.locales"), configs.cfgman->getString("core.default_locale"));
  }

  if(smsc_app_runner->getApp()->isHs()){
      Interconnect * iconn = Interconnect::getInstance();
      if(!iconn){
          smsc_log_warn(logger, "InterconnectManager undefined, can't send applyRoutes command");
          //return;
      }else{
          ApplyLocaleResourceCommand * cmd = new ApplyLocaleResourceCommand();
          iconn->sendCommand(cmd);
      }
  }

  return Variant("");
}


void SmscComponent::applyTimeZones()throw(AdminException)
{
  try{
    smsc::system::common::TimeZoneManager::getInstance().Reload();
  }catch(std::exception& e)
  {
    throw AdminException("%s",e.what());
  }
}

#ifdef USE_MAP
void SmscComponent::applyFraudControl()throw(AdminException)
{
  try{
    smsc::system::mapio::FraudControl::getInstance()->Reload();
  }catch(std::exception& e)
  {
    throw AdminException("FraudControl::Reload - %s",e.what());
  }
}

void SmscComponent::applyMapLimits()throw(AdminException)
{
  try
  {
    smsc::system::mapio::MapLimits::getInstance().Reinit();
  } catch(std::exception& e)
  {
    throw AdminException("MapLimits::Reinit - %s",e.what());
  }
}
#endif

Variant SmscComponent::aclListNames(const Arguments & args) throw (AdminException)
{
  AclAbstractMgr   *aclmgr = smsc_app_runner->getApp()->getAclMgr();
  typedef std::vector<AclNamedIdent> AclNames;
  AclNames resultList;
  try {
    aclmgr->enumerate(resultList);
  } catch (std::exception &e) {
    throw AdminException("Could not enumerate ACLs, nested: \"%s\"", e.what());
  } catch (const char * const e) {
    throw AdminException("Could not enumerate ACLs, nested: \"%s\"", e);
  } catch (...) {
    throw AdminException("Could not enumerate ACLs, nested: Unknown exception");
  }

  Variant result(service::StringListType);
  for (AclNames::const_iterator i = resultList.begin(); i != resultList.end(); i++)
  {
    const AclNamedIdent & ident(*i);
    const size_t len = 3*sizeof(ident.first) + ident.second.length()+2;
    std::auto_ptr<char> buffer(new char[len]);
    snprintf(buffer.get(), len, "%lu,%s", ident.first, ident.second.c_str());
    result.appendValueToStringList(buffer.get());
  }

  return result;
}

Variant SmscComponent::aclGet(const Arguments & args) throw (AdminException)
{
  try {
    AclIdent aclId = (AclIdent)args.Get("id").getLongValue();
    AclAbstractMgr   *aclmgr = smsc_app_runner->getApp()->getAclMgr();

    AclInfo aclInfo = aclmgr->getInfo(aclId);

    Variant result(service::StringListType);
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%lu", aclInfo.ident);
    result.appendValueToStringList(buffer);
    result.appendValueToStringList(aclInfo.name);
    result.appendValueToStringList(aclInfo.description);
    buffer[0] = aclInfo.cache;
    buffer[1] = 0;
    result.appendValueToStringList(buffer);
    return result;
  } catch (HashInvalidKeyException &e) {
    throw new AdminException("Parameter id not found");
  } catch (std::exception &e) {
    throw AdminException("Could not get ACL info, nested: \"%s\"", e.what());
  } catch (const char * const e) {
    throw AdminException("Could not get ACL info, nested: \"%s\"", e);
  } catch (...) {
    throw AdminException("Could not get ACL info, nested: Unknown exception");
  }
}

Variant SmscComponent::aclRemove(const Arguments & args) throw (AdminException)
{
  try {
    AclIdent aclId = (AclIdent)args.Get("id").getLongValue();
    AclAbstractMgr   *aclmgr = smsc_app_runner->getApp()->getAclMgr();
    try {
      aclmgr->remove(aclId);
    } catch (std::exception &e) {
      throw AdminException("Could not remove ACL, nested: \"%s\"", e.what());
    } catch (const char * const e) {
      throw AdminException("Could not remove ACL, nested: \"%s\"", e);
    } catch (...) {
      throw AdminException("Could not remove ACL, nested: Unknown exception");
    }

    Variant result("removed");
    return result;
  } catch (HashInvalidKeyException &e) {
    throw AdminException("Parameter id not found");
  }
}

Variant SmscComponent::aclCreate(const Arguments & args) throw (AdminException)
{
  try {
    const char * const name = args.Get("name").getStringValue();
    const char * const description = args.Get("description").getStringValue();
    const char * const cache_type_str = args.Get("cache_type").getStringValue();
    const bool cache_type_present = (cache_type_str != NULL) && (cache_type_str[0] != 0);

    const StringList & addresses(args.Get("addresses").getStringListValue());
    std::vector<AclPhoneNumber> phones;
    for (StringList::const_iterator i = addresses.begin(); i != addresses.end(); i++)
    {
      const char * const addrStr = *i;
      if (addrStr != NULL && addrStr[0] != 0)
      {
        std::string address(addrStr);
        phones.push_back(address);
      }
    }

    try {
      AclAbstractMgr   *aclmgr = smsc_app_runner->getApp()->getAclMgr();
      if (cache_type_present)
        return Variant((const long)aclmgr->create2(name, description, phones, (AclCacheType)(cache_type_str[0])));
      else
        return Variant((const long)aclmgr->create2(name, description, phones));
    } catch (std::exception &e) {
      throw AdminException("Could not create ACL, nested: \"%s\"", e.what());
    } catch (const char * const e) {
      throw AdminException("Could not create ACL, nested: \"%s\"", e);
    } catch (...) {
      throw AdminException("Could not create ACL, nested: Unknown exception");
    }
  } catch (HashInvalidKeyException &e) {
    throw AdminException("Parameter id or name or description or adressess not found");
  }
}

Variant SmscComponent::aclUpdateInfo(const Arguments & args) throw (AdminException)
{
  try {
    AclIdent aclId = (AclIdent)args.Get("id").getLongValue();
    const char * const name = args.Get("name").getStringValue();
    const char * const description = args.Get("description").getStringValue();
    const char * const cache_type_str = args.Get("cache_type").getStringValue();
    const bool cache_type_present = (cache_type_str != NULL) && (cache_type_str[0] != 0);

    try {
      AclAbstractMgr   *aclmgr = smsc_app_runner->getApp()->getAclMgr();
      if (cache_type_present)
        aclmgr->updateAclInfo(aclId, name, description, (AclCacheType)(cache_type_str[0]));
      else
        aclmgr->updateAclInfo(aclId, name, description);
    } catch (std::exception &e) {
      throw AdminException("Could not update ACL info, nested: \"%s\"", e.what());
    } catch (const char * const e) {
      throw AdminException("Could not update ACL info, nested: \"%s\"", e);
    } catch (...) {
      throw AdminException("Could not update ACL info, nested: Unknown exception");
    }

    Variant result("updated");
    return result;
  } catch (HashInvalidKeyException &e) {
    throw AdminException("Parameter id or name or description not found");
  }
}

Variant SmscComponent::aclLookupAddresses(const Arguments & args) throw (AdminException)
{
  try {
    AclIdent aclId = (AclIdent)args.Get("id").getLongValue();
    const char * const prefix = args.Get("prefix").getStringValue();

    typedef std::vector<AclPhoneNumber> Phones;
    Phones resultPhones;

    try {
      AclAbstractMgr   *aclmgr = smsc_app_runner->getApp()->getAclMgr();
      aclmgr->lookupByPrefix(aclId, prefix, resultPhones);
    } catch (std::exception &e) {
      throw AdminException("Could not lookup addresses, nested: \"%s\"", e.what());
    } catch (const char * const e) {
      throw AdminException("Could not lookup addresses, nested: \"%s\"", e);
    } catch (...) {
      throw AdminException("Could not lookup addresses, nested: Unknown exception");
    }

    Variant result(service::StringListType);
    for (Phones::const_iterator i = resultPhones.begin(); i != resultPhones.end(); i++) {
      result.appendValueToStringList(*i);
    }
    return result;
  } catch (HashInvalidKeyException &e) {
    throw AdminException("Parameter id or prefix not found");
  }
}

Variant SmscComponent::aclRemoveAddresses(const Arguments & args) throw (AdminException)
{
  try {
    AclIdent aclId = (AclIdent)args.Get("id").getLongValue();
    const StringList & addresses(args.Get("addresses").getStringListValue());

    try {
      AclAbstractMgr   *aclmgr = smsc_app_runner->getApp()->getAclMgr();
      for (StringList::const_iterator i = addresses.begin(); i != addresses.end(); i++) {
        std::string address(*i);
        aclmgr->removePhone(aclId, address);
      }
    } catch (std::exception &e) {
      throw AdminException("Could not remove addresses, nested: \"%s\"", e.what());
    } catch (const char * const e) {
      throw AdminException("Could not remove addresses, nested: \"%s\"", e);
    } catch (...) {
      throw AdminException("Could not remove addresses, nested: Unknown exception");
    }

    Variant result("removed addresses");
    return result;
  } catch (HashInvalidKeyException &e) {
    throw AdminException("Parameter id or adressess not found");
  }
}

Variant SmscComponent::aclAddAddresses(const Arguments & args) throw (AdminException)
{
  try {
    AclIdent aclId = (AclIdent)args.Get("id").getLongValue();
    const StringList & addresses(args.Get("addresses").getStringListValue());

    try {
      AclAbstractMgr   *aclmgr = smsc_app_runner->getApp()->getAclMgr();
      for (StringList::const_iterator i = addresses.begin(); i != addresses.end(); i++) {
        std::string address(*i);
        aclmgr->addPhone(aclId, address);
      }
    } catch (std::exception &e) {
      throw AdminException("Could not add addresses, nested: \"%s\"", e.what());
    } catch (const char * const e) {
      throw AdminException("Could not add addresses, nested: \"%s\"", e);
    } catch (...) {
      throw AdminException("Could not add addresses, nested: Unknown exception");
    }

    Variant result("added addresses");
    return result;
  } catch (HashInvalidKeyException &e) {
    throw AdminException("Parameter id or adressess not found");
  }
}

using namespace smsc::distrlist;

#define PROLOGUE \
  info2(logger,"dladmin call %s",__func__); \
  DistrListAdmin* dladmin=smsc_app_runner->getApp()->getDlAdmin();\
  const char* lastArg=0; \
  try{

#define BEGINMETHOD\
  try

#define INTARG(name) lastArg=#name;int name=(int)args.Get(#name).getLongValue();
#define STRARG(name) lastArg=#name;const char* name=args.Get(#name).getStringValue();
#define LSTARG(name) lastArg=#name;const StringList& name=args.Get(#name).getStringListValue();

#define  ENDMETHOD \
  catch(std::exception& e) \
  { \
    throw AdminException("%s failed:\"%s\"",__func__,e.what());\
  } \
  catch(const char* e) \
  { \
    throw AdminException("%s failed:\"%s\"",__func__,e);\
  } \
  catch(...) \
  { \
    throw AdminException("%s failed: Unknown exception",__func__);\
  }
#define EPILOGUE \
  }catch(HashInvalidKeyException& e) \
  { \
    throw AdminException("Parameter %s not found in args",lastArg?lastArg:"unknown"); \
  }


Variant SmscComponent::dlListPrincipals(const Arguments & args) throw (AdminException)
{
  PROLOGUE
    STRARG(address);
    Variant result(service::StringListType);
  BEGINMETHOD
  {
    Array<Principal> prcs=dladmin->getPrincipals();
    char buf[32];
    size_t addrLen=address?strlen(address):0;
    for(int i=0;i<prcs.Count();i++)
    {
      Principal& prc=prcs[i];
      size_t bl=prc.address.toString(buf,sizeof(buf));
      if(addrLen && strncmp(address,buf,std::min(addrLen,bl))!=0)continue;
      prc.address.toString(buf,sizeof(buf));
      result.appendValueToStringList(buf);
      sprintf(buf,"%d",prc.maxLst);
      result.appendValueToStringList(buf);
      sprintf(buf,"%d",prc.maxEl);
      result.appendValueToStringList(buf);
    }
  }
  ENDMETHOD
    return result;
  EPILOGUE
}


Variant SmscComponent::dlAddPrincipal(const Arguments & args) throw (AdminException)
{
  PROLOGUE
    STRARG(address);
    INTARG(maxLists);
    INTARG(maxElements);
  BEGINMETHOD
  {
    dladmin->addPrincipal(Principal(address,maxLists,maxElements));
  }
  ENDMETHOD
    return Variant("Principal added");
  EPILOGUE
}


Variant SmscComponent::dlDelPrincipal(const Arguments & args) throw (AdminException)
{
  PROLOGUE
    STRARG(address);
  BEGINMETHOD
  {
    dladmin->deletePrincipal(address);
  }
  ENDMETHOD
    return Variant("Principal deleted");
  EPILOGUE
}


Variant SmscComponent::dlGetPrincipal(const Arguments & args) throw (AdminException)
{
  PROLOGUE
    STRARG(address);
    Variant result(service::StringListType);
  BEGINMETHOD
  {
    Principal prc=dladmin->getPrincipal(address);
    result.appendValueToStringList(prc.address.toString().c_str());
    char buf[32];
    sprintf(buf,"%d",prc.maxLst);
    result.appendValueToStringList(buf);
    sprintf(buf,"%d",prc.maxEl);
    result.appendValueToStringList(buf);
  }
  ENDMETHOD
    return result;
  EPILOGUE
}


Variant SmscComponent::dlAlterPrincipal(const Arguments & args) throw (AdminException)
{
  PROLOGUE
    STRARG(address);
    INTARG(maxLists);
    INTARG(maxElements);
  BEGINMETHOD
  {
    dladmin->changePrincipal(Principal(address,maxLists,maxElements));
  }
  ENDMETHOD
    return Variant("Principal altered");
  EPILOGUE
}



Variant SmscComponent::dlAddMember(const Arguments & args) throw (AdminException)
{
  PROLOGUE
    STRARG(dlname);
    STRARG(address);
  BEGINMETHOD
  {
    dladmin->addMember(dlname,address);
  }
  ENDMETHOD
    return Variant("Member added");

  EPILOGUE
}


Variant SmscComponent::dlDeleteMember(const Arguments & args) throw (AdminException)
{
  PROLOGUE
    STRARG(dlname);
    STRARG(address);
  BEGINMETHOD
  {
    dladmin->deleteMember(dlname,address);
  }
  ENDMETHOD
    return Variant("Member deleted");

  EPILOGUE
}


Variant SmscComponent::dlGetMembers(const Arguments & args) throw (AdminException)
{
  PROLOGUE
    STRARG(dlname);
    Variant result(service::StringListType);
  BEGINMETHOD
  {
    Array<Address> members=dladmin->members(dlname);
    for(int i=0;i<members.Count();i++)
    {
      result.appendValueToStringList(members[i].toString().c_str());
    }
  }
  ENDMETHOD
    return result;
  EPILOGUE
}



Variant SmscComponent::dlAddSubmitter(const Arguments & args) throw (AdminException)
{
  PROLOGUE
    STRARG(dlname);
    STRARG(address);
  BEGINMETHOD
  {
    dladmin->grantPosting(dlname,address);
  }
  ENDMETHOD
    return Variant("Submitter added");

  EPILOGUE
}


Variant SmscComponent::dlDeleteSubmitter(const Arguments & args) throw (AdminException)
{
  PROLOGUE
    STRARG(dlname);
    STRARG(address);
  BEGINMETHOD
  {
    dladmin->revokePosting(dlname,address);
  }
  ENDMETHOD
    return Variant("Submitter deleted");

  EPILOGUE
}


Variant SmscComponent::dlListSubmitters(const Arguments & args) throw (AdminException)
{
  PROLOGUE
    STRARG(dlname);
    Variant result(service::StringListType);
  BEGINMETHOD
  {
    Array<Address> sbm;
    dladmin->getSubmitters(dlname,sbm);
    char buf[32];
    for(int i=0;i<sbm.Count();i++)
    {
      sbm[i].toString(buf,sizeof(buf));
      result.appendValueToStringList(buf);
      result.appendValueToStringList("0");
      result.appendValueToStringList("0");
    }
  }
  ENDMETHOD
    return result;
  EPILOGUE
}



Variant SmscComponent::dlAddList(const Arguments & args) throw (AdminException)
{
  PROLOGUE
    STRARG(dlname);
    STRARG(owner);
    INTARG(maxElements);
  BEGINMETHOD
  {
    dladmin->addDistrList(dlname,!owner || !*owner,owner && *owner?owner:"0",maxElements);
  }
  ENDMETHOD
    return Variant("list added");
  EPILOGUE
}


Variant SmscComponent::dlDeleteList(const Arguments & args) throw (AdminException)
{
  PROLOGUE
    STRARG(dlname);
  BEGINMETHOD
  {
    dladmin->deleteDistrList(dlname);
  }
  ENDMETHOD
    return Variant("list deleted");

  EPILOGUE
}


Variant SmscComponent::dlGetList(const Arguments & args) throw (AdminException)
{
  PROLOGUE
    STRARG(dlname);
    Variant result(service::StringListType);
  BEGINMETHOD
  {
    smsc::distrlist::DistrList lst=dladmin->getDistrList(dlname);
    result.appendValueToStringList(lst.name);
    result.appendValueToStringList(lst.system?"":lst.owner.toString().c_str());
    char buf[32];
    sprintf(buf,"%d",lst.maxEl);
    result.appendValueToStringList(buf);
  }
  ENDMETHOD
    return result;

  EPILOGUE
}


Variant SmscComponent::dlListLists(const Arguments & args) throw (AdminException)
{
  PROLOGUE
    LSTARG(names);
    LSTARG(owners);
    Variant result(service::StringListType);
  BEGINMETHOD
  {
    Array<smsc::distrlist::DistrList> lst=dladmin->list();
    Hash<int> nmflt;
    Hash<int> ownflt;
    bool haveNmFlt=false;
    bool haveOwnFlt=false;
    if(!names.empty())
    {
      for(StringList::const_iterator it=names.begin();it!=names.end();it++)
      {
        if(!**it)continue;
        nmflt.Insert(*it,1);
        haveNmFlt=true;
      }

    }
    if(!owners.empty())
    {
      for(StringList::const_iterator it=owners.begin();it!=owners.end();it++)
      {
        if(!**it)continue;
        ownflt.Insert(*it,1);
        haveOwnFlt=true;
      }
    }
    char buf[32];
    for(int i=0;i<lst.Count();i++)
    {
      smsc::distrlist::DistrList& dl=lst[i];
      if(haveNmFlt && !nmflt.Exists(dl.name))continue;
      dl.owner.getText(buf,sizeof(buf));
      if(haveOwnFlt && (dl.system || !ownflt.Exists(buf)))continue;
      result.appendValueToStringList(dl.name);
      result.appendValueToStringList(dl.system?"":buf);
      sprintf(buf,"%d",dl.maxEl);
      result.appendValueToStringList(buf);
    }
  }
  ENDMETHOD
    return result;
  EPILOGUE
}


Variant SmscComponent::dlAlterList(const Arguments & args) throw (AdminException)
{
  PROLOGUE
    STRARG(dlname);
    INTARG(maxElements);
  BEGINMETHOD
  {
    dladmin->changeDistrList(dlname,maxElements);
  }
  ENDMETHOD
    return Variant("list altered");
  EPILOGUE
}

Variant SmscComponent::dlCopyList(const Arguments& args) throw (AdminException)
{
  PROLOGUE
    STRARG(dlname);
    STRARG(newdlname);
  BEGINMETHOD
  {
    dladmin->copyDistrList(dlname,newdlname);
  }
  ENDMETHOD
    return Variant("copy ok");
  EPILOGUE
}

Variant SmscComponent::dlRenameList(const Arguments& args) throw (AdminException)
{
  PROLOGUE
    STRARG(dlname);
    STRARG(newdlname);
  BEGINMETHOD
  {
    dladmin->renameDistrList(dlname,newdlname);
  }
  ENDMETHOD
    return Variant("rename ok");
  EPILOGUE
}


Variant SmscComponent::setRole(const Arguments & args) throw (AdminException)
{

    Interconnect *icon = 0;

    try {

        icon = Interconnect::getInstance();

        const char * const role = args.Get("role").getStringValue();

        if(strcmp(role, "MASTER") == 0){
            icon->changeRole(smsc::cluster::MASTER);
        }else if(strcmp(role, "SLAVE") == 0){
            icon->changeRole(smsc::cluster::SLAVE);
        }else if(strcmp(role, "SINGLE") == 0){
            icon->changeRole(smsc::cluster::SINGLE);
        }

    }catch(...){
        throw Exception("Exception during setRole method");
    }

    return Variant("");

}

Variant SmscComponent::getRole() throw (AdminException)
{

    Interconnect *icon = 0;

    try {

        icon = Interconnect::getInstance();

        smsc::cluster::Role role = icon->getRole();

        switch(role){
        case smsc::cluster::MASTER:
            return Variant("MASTER");
        case smsc::cluster::SLAVE:
            return Variant("SLAVE");
        case smsc::cluster::SINGLE:
            return Variant("SINGLE");
        default:
            return Variant("UNKNOWN");
        }

    }catch(...){
        throw Exception("Exception during getRole method");
    }

    return Variant("UNKNOWN");

}

#define CGMPROLOGUE \
  info2(logger,"cgm call %s",__func__); \
  smsc::closedgroups::ClosedGroupsInterface* cgm=smsc::closedgroups::ClosedGroupsInterface::getInstance();\
  const char* lastArg=0; \
  try{


Variant SmscComponent::cgmAddGroup(const Arguments & args)
{
  CGMPROLOGUE
    INTARG(id);
    STRARG(name);
  BEGINMETHOD
  {
    cgm->AddGroup(id,name);
  }
  ENDMETHOD
    return Variant("group added");
  EPILOGUE
}
Variant SmscComponent::cgmDeleteGroup(const Arguments & args)
{
  CGMPROLOGUE
    INTARG(id);
  BEGINMETHOD
  {
    cgm->DeleteGroup(id);
  }
  ENDMETHOD
    return Variant("group deleted");
  EPILOGUE
}
Variant SmscComponent::cgmAddAddr(const Arguments & args)
{
  CGMPROLOGUE
    INTARG(id);
    STRARG(addr);
  BEGINMETHOD
  {
    cgm->AddAddrToGroup(id,addr);
  }
  ENDMETHOD
    return Variant("address added");
  EPILOGUE
}
Variant SmscComponent::cgmCheck(const Arguments & args)
{
  CGMPROLOGUE
    INTARG(id);
    STRARG(addr);
    bool result;
  BEGINMETHOD
  {
    result=cgm->Check(id,addr);
  }
  ENDMETHOD
    return Variant(result);
  EPILOGUE
}
Variant SmscComponent::cgmDelAddr(const Arguments & args)
{
  CGMPROLOGUE
    INTARG(id);
    STRARG(addr);
  BEGINMETHOD
  {
    cgm->RemoveAddrFromGroup(id,addr);
  }
  ENDMETHOD
    return Variant("address removed");
  EPILOGUE
}
Variant SmscComponent::cgmAddAbonent(const Arguments & args)
{
  CGMPROLOGUE
    INTARG(id);
    STRARG(addr);
  BEGINMETHOD
  {
    cgm->AddAbonent(id,addr);
  }
  ENDMETHOD
    return Variant("abonent added");
  EPILOGUE
}
Variant SmscComponent::cgmDelAbonent(const Arguments & args)
{
  CGMPROLOGUE
    INTARG(id);
    STRARG(addr);
  BEGINMETHOD
  {
    cgm->RemoveAbonent(id,addr);
  }
  ENDMETHOD
    return Variant("abonent removed");
  EPILOGUE
}
Variant SmscComponent::cgmListAbonents(const Arguments & args)
{
  CGMPROLOGUE
    INTARG(id);
    Variant result(service::StringListType);
  BEGINMETHOD
  {
    std::vector<Address> list;
    cgm->ListAbonents(id,list);
    for(std::vector<Address>::iterator it=list.begin();it!=list.end();it++)
    {
      result.appendValueToStringList(it->toString().c_str());
    }
  }
  ENDMETHOD
    return result;
  EPILOGUE
}

#define ALIASPROLOGUE \
  info2(logger,"alias admin call %s",__func__); \
  AliasManager* aliaser=smsc_app_runner->getApp()->getAliaserInstance();\
  const char* lastArg=0; \
  try{


Variant SmscComponent::addAlias(const Arguments & args)
{
  ALIASPROLOGUE
    STRARG(address);
    STRARG(alias);
    INTARG(hide);
  BEGINMETHOD
  {
    smsc::alias::AliasInfo ai;
    ai.addr=address;
    ai.alias=alias;
    ai.hide=hide;
    aliaser->addAlias(ai);
  }
  ENDMETHOD
    return Variant("alias added");
  EPILOGUE
}

Variant SmscComponent::delAlias(const Arguments & args)
{
  ALIASPROLOGUE
    STRARG(alias);
  BEGINMETHOD
  {
    aliaser->deleteAlias(alias);
  }
  ENDMETHOD
    return Variant("alias deleted");
  EPILOGUE
}

#define PROFILERPROLOGUE \
  info2(logger,"profiler admin call %s",__func__); \
  ProfilerInterface* profiler=smsc_app_runner->getApp()->getProfiler();\
  const char* lastArg=0; \
  try{


#ifdef SMSEXTRA
void SmscComponent::setSponsoredValue(const Arguments& args)
{
  PROFILERPROLOGUE
    LSTARG(list);
  BEGINMETHOD
  {
    StringList::const_iterator it=list.begin();
    while(it!=list.end())
    {
      const char* abonent=*it;
      it++;
      if(it==list.end())
      {
        smsc_log_warn(logger,"Invalid amount of arguments for setSponsoredValue");
        break;
      }
      const char* value=*it;
      it++;
      Profile p=profiler->lookup(abonent);
      p.sponsored=atoi(value);
      profiler->update(abonent,p);
    }
  }
  ENDMETHOD
    return;
  EPILOGUE
}
#endif


#ifdef SNMP

void SmscComponent::applySnmp()
{
  try
  {
    smsc::system::SnmpCounter::getInstance().ReloadCfg();
  }catch(std::exception& e)
  {
    throw AdminException("SnmpApplyConfigException:%s",e.what());
  }
}
#endif

}
}
}
