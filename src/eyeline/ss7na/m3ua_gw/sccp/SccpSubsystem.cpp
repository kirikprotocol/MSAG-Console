#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>

#include "util/Exception.hpp"
#include "eyeline/utilx/strtol.hpp"
#include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"

#include "eyeline/ss7na/common/sccp_sap/ProtocolStates.hpp"
#include "eyeline/ss7na/common/sccp_sap/ActiveAppsRegistry.hpp"

#include "eyeline/ss7na/m3ua_gw/io_dispatcher/ConnectMgr.hpp"
#include "eyeline/ss7na/m3ua_gw/runtime_cfg/RuntimeConfig.hpp"

#include "initializer.hpp"
#include "MTP3IndicationsProcessor.hpp"
#include "ReassemblyProcessor.hpp"
#include "SccpSubsystem.hpp"
#include "MessagesFactory.hpp"

#include "router/GTTranslationTablesRegistry.hpp"

#include "sap/MessageHandlers.hpp"
#include "sap/initializer.hpp"
#include "sap/ApplicationsRegistry.hpp"
#include "sap/LibSccpConnectAcceptor.hpp"
#include "PolicyRegistry.hpp"
#include "MTP3SapSelectPolicy.hpp"
#include "scmg/initializer.hpp"
#include "scmg/MessagesFactory.hpp"
#include "scmg/SCMGMessageProcessor.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {

utilx::ThreadSpecificData<bool>
SccpSubsystem::_threadScopeModificationFlag;

SccpSubsystem::SccpSubsystem()
  : common::ApplicationSubsystem("SccpSubsystem", "sccp")
{
  memset(_acceptorName, 0, sizeof(_acceptorName));
}

void
SccpSubsystem::stop()
{
  smsc_log_info(_logger, "SccpSubsystem::stop::: shutdown subsystem");
  delete io_dispatcher::ConnectMgr::getInstance().removeConnectAcceptor(_acceptorName, true);
}

void
SccpSubsystem::initialize(utilx::runtime_cfg::RuntimeConfig& rconfig)
{
  smsc_log_info(_logger, "SccpSubsystem::initialize::: try initialize SccpSubsystem");

  common::sccp_sap::LibSccp_State_NoConnection::init();
  common::sccp_sap::LibSccp_State_Unbind::init();
  common::sccp_sap::LibSccp_State_Bind::init();
  router::GTTranslationTablesRegistry::init();
  router::GTTranslator::init();
  sap::ApplicationsRegistry::init();
  scmg::MessagesFactory::init();
  MessagesFactory::init();

  common::sccp_sap::ActiveAppsRegistry::init();
  MTP3IndicationsProcessor::init();
  ReassemblyProcessor::init();

  registerMessageCreators();
  scmg::registerMessageCreators();
  scmg::SCMGMessageProcessor::init();
  PolicyRegistry<MTP3SapSelectPolicy>::init();
  PolicyRegistry<SCCPUserSelectPolicy>::init();

  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.commit", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.local_address", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.local_port", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.sccp_users", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.sccp_users.commit", this);
  runtime_cfg::RuntimeConfig::getInstance().registerParameterObserver("config.sccp_users.userid", this);

  utilx::runtime_cfg::Parameter& listeningHostParameter = rconfig.find<utilx::runtime_cfg::Parameter>("config.local_address");
  utilx::runtime_cfg::Parameter& listeningPortParameter = rconfig.find<utilx::runtime_cfg::Parameter>("config.local_port");

  std::string localHost = listeningHostParameter.getValue();
  in_port_t localPort = (in_port_t)utilx::strtol(listeningPortParameter.getValue().c_str(), (char **)NULL, 10);
  if ( localPort == 0 && errno )
    throw smsc::util::Exception("SccpSubsystem::initialize::: invalid config.local_port value='%s'",
                                listeningPortParameter.getValue().c_str());

  utilx::runtime_cfg::CompositeParameter& sccpUsers = rconfig.find<utilx::runtime_cfg::CompositeParameter>("config.sccp_users");

  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::Parameter> iterator = sccpUsers.getIterator<utilx::runtime_cfg::Parameter>("userid");
  //iterator points to first element. If there are nothing elements in result then hasElement() returns false
  while(iterator.hasElement()) {
    const utilx::runtime_cfg::Parameter* parameter = iterator.getCurrentElement();
    sap::ApplicationsRegistry::getInstance().insert(parameter->getValue() /*userId*/);
    smsc_log_info(_logger, "SccpSubsystem::initialize::: insert application id='%s' into ApplicationsRegistry",
                  parameter->getValue().c_str());
    iterator.next();
  }
  sap::registerMessageCreators();

  fillInTranslationTable(rconfig);
  initializeConnectAcceptor(localHost, localPort);

  utilx::runtime_cfg::Parameter& reassemblyTimerParameter = rconfig.find<utilx::runtime_cfg::Parameter>("config.reassembly_timer");
  unsigned timerValue = (unsigned)utilx::strtol(reassemblyTimerParameter.getValue().c_str(), (char **)NULL, 10);
  if ( timerValue == 0 && errno )
    throw smsc::util::Exception("SccpSubsystem::initialize::: invalid config.reassembly_timer value='%s'",
                                reassemblyTimerParameter.getValue().c_str());

  ReassemblyProcessor::getInstance().initialize(timerValue);
}

void
SccpSubsystem::addParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                                           utilx::runtime_cfg::Parameter* added_parameter)
{
  if ( context.getFullName() == "config.sua_applications" ) {
    utilx::runtime_cfg::RuntimeConfig& runtimeConfig = runtime_cfg::RuntimeConfig::getInstance();
    utilx::runtime_cfg::CompositeParameter& suaApplicationsParam = runtimeConfig.find<utilx::runtime_cfg::CompositeParameter>("config.sua_applications");

    if ( added_parameter->getName() == "application" ) {
      smsc_log_debug(_logger, "SccpSubsystem::addParameterEventHandler::: handle added parameter 'config.sua_applications.application'='%s'", added_parameter->getValue().c_str());

      if ( !checkParameterExist(&suaApplicationsParam, added_parameter) )
        suaApplicationsParam.addParameter(added_parameter);
    }
  }
}

void
SccpSubsystem::changeParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                                              const utilx::runtime_cfg::Parameter& modified_parameter)
{
  if ( context.getFullName() == "config" ) {
    utilx::runtime_cfg::RuntimeConfig& runtimeConfig = runtime_cfg::RuntimeConfig::getInstance();
    utilx::runtime_cfg::CompositeParameter& rootConfigParam = runtimeConfig.find<utilx::runtime_cfg::CompositeParameter>("config");

    if ( modified_parameter.getName() == "commit" )
      applyParametersChange();
    else {
      if ( modified_parameter.getName() != "local_address" &&
           modified_parameter.getName() != "local_port" )
        return;

      smsc_log_debug(_logger, "SccpSubsystem::changeParameterEventHandler::: handle modified parameter 'config.%s'='%s'", modified_parameter.getName().c_str(), modified_parameter.getValue().c_str());
      utilx::runtime_cfg::Parameter* foundParam = rootConfigParam.getParameter<utilx::runtime_cfg::Parameter>(modified_parameter.getName());
      if ( !foundParam )
        throw smsc::util::Exception("SccpSubsystem::changeParameterEventHandler::: parameter '%s' not found in current configuration", modified_parameter.getName().c_str());

      foundParam->setValue(modified_parameter.getValue());
      _threadScopeModificationFlag.set(true);
    }
  }
}

void
SccpSubsystem::applyParametersChange()
{
  smsc_log_debug(_logger, "SccpSubsystem::applyParametersChange::: Enter it");

  if ( !_threadScopeModificationFlag.get(false) ) return;

  _threadScopeModificationFlag.set(false);

  utilx::runtime_cfg::RuntimeConfig& rconfig = runtime_cfg::RuntimeConfig::getInstance();

  utilx::runtime_cfg::Parameter& listeningHostParameter = rconfig.find<utilx::runtime_cfg::Parameter>("config.local_address");
  utilx::runtime_cfg::Parameter& listeningPortParameter = rconfig.find<utilx::runtime_cfg::Parameter>("config.local_port");

  std::string localHost = listeningHostParameter.getValue();
  in_port_t localPort = (in_port_t)utilx::strtol(listeningPortParameter.getValue().c_str(), (char **)NULL, 10);
  if ( localPort == 0 && errno )
    throw smsc::util::Exception("SccpSubsystem::applyParametersChange::: invalid config.local_port value");

  delete io_dispatcher::ConnectMgr::getInstance().removeConnectAcceptor(_acceptorName, false); // false --> don't remove current active client's connections
  initializeConnectAcceptor(localHost, localPort);
}

void
SccpSubsystem::fillInTranslationTable(utilx::runtime_cfg::RuntimeConfig& rconfig)
{
  router::GTTranslationTablesRegistry::getInstance().registerGTTranslationTable(".4.0.1.4",
                                                                                new router::GTTranslationTable_Prefix_4_0_1_4());
  router::GTTranslationTablesRegistry::getInstance().registerGTTranslationTable(".4.0.1.0",
                                                                                new router::GTTranslationTable_Prefix_4_0_1_0());

  utilx::runtime_cfg::CompositeParameter& translationTableParameter = rconfig.find<utilx::runtime_cfg::CompositeParameter>("config.translation-table");
  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter>
    entryIterator = translationTableParameter.getIterator<utilx::runtime_cfg::CompositeParameter>("entry");
  while (entryIterator.hasElement()) {
    utilx::runtime_cfg::CompositeParameter* nextParameter = entryIterator.getCurrentElement();

    const std::string& gt = nextParameter->getParameter<utilx::runtime_cfg::Parameter>("gt")->getValue();
    const std::string& gtAddrFamilyPrefix = makeAddressFamilyPrefix(gt);
    router::GTTranslationTable* translationTable = router::GTTranslationTablesRegistry::getInstance().getGTTranslationTable(gtAddrFamilyPrefix);
    if ( !translationTable )
      throw smsc::util::Exception("MessagesRouterSubsystem::addRouteEntry::: translation table not found for address family=[%s]",
                                  gtAddrFamilyPrefix.c_str());

    utilx::runtime_cfg::CompositeParameter* sccpUsersParam =
        nextParameter->getParameter<utilx::runtime_cfg::CompositeParameter>("sccp_users");
    if ( sccpUsersParam )
      processRouteInfoToSccpUser(nextParameter, translationTable, sccpUsersParam);
    else
      processRouteInfoToMTP3(nextParameter, translationTable);

    entryIterator.next();
  }
}

void
SccpSubsystem::processRouteInfoToSccpUser(utilx::runtime_cfg::CompositeParameter* translation_entry_param,
                                          router::GTTranslationTable* translation_table,
                                          utilx::runtime_cfg::CompositeParameter* sccp_users_param)
{
  const std::string& routeId = translation_entry_param->getValue(); // e.g. MasterGT, CLOUD

  utilx::runtime_cfg::Parameter* trafficModeParam =
      translation_entry_param->getParameter<utilx::runtime_cfg::Parameter>("traffic-mode");
  if ( !trafficModeParam )
    throw smsc::util::Exception("SccpSubsystem::processRouteInfoToSccpUser::: parameter config.translation-table.entry.traffic-mode is absent");

  const std::string& trafficMode = trafficModeParam->getValue();
  const std::string& gt = translation_entry_param->getParameter<utilx::runtime_cfg::Parameter>("gt")->getValue();
  utilx::runtime_cfg::Parameter* ssnParam = translation_entry_param->getParameter<utilx::runtime_cfg::Parameter>("ssn");
  if ( ssnParam ) {
    uint8_t ssn = static_cast<uint8_t>(utilx::strtol(ssnParam->getValue().c_str(), NULL, 10));
    if ( ssn == 0 && errno )
      throw smsc::util::Exception("SccpSubsystem::processRouteInfoToSccpUser::: invalid value for parameter config.translation-table.entry.ssn");
    translation_table->addTranslationEntry(gt, ssn, routeId);
  } else
    translation_table->addTranslationEntry(gt, routeId, false);

  addSccpUserPolicy(routeId, sccp_users_param, trafficMode);
}

void
SccpSubsystem::processRouteInfoToMTP3(utilx::runtime_cfg::CompositeParameter* translation_entry_param,
                                      router::GTTranslationTable* translation_table)
{
  const std::string& routeId = translation_entry_param->getValue(); // e.g. MasterGT, CLOUD

  utilx::runtime_cfg::Parameter* trafficModeParam =
      translation_entry_param->getParameter<utilx::runtime_cfg::Parameter>("traffic-mode");
  if ( !trafficModeParam )
    throw smsc::util::Exception("SccpSubsystem::processRouteInfoToMTP3::: parameter config.translation-table.entry.traffic-mode is absent");

  const std::string& trafficMode = trafficModeParam->getValue();
  const std::string& gt = translation_entry_param->getParameter<utilx::runtime_cfg::Parameter>("gt")->getValue();
  utilx::runtime_cfg::CompositeParameter* dpcsParam =
      translation_entry_param->getParameter<utilx::runtime_cfg::CompositeParameter>("dpcs");
  if ( !dpcsParam )
    throw smsc::util::Exception("SccpSubsystem::processRouteInfoToMTP3::: parameter config.translation-table.entry.dpcs is absent");

  utilx::runtime_cfg::Parameter* lpcParam =
      translation_entry_param->getParameter<utilx::runtime_cfg::Parameter>("lpc");
  if ( !lpcParam )
    throw smsc::util::Exception("SccpSubsystem::processRouteInfoToMTP3::: parameter config.translation-table.entry.lpc is absent");

  common::point_code_t lpc = static_cast<common::point_code_t>(utilx::strtol(lpcParam->getValue().c_str(), NULL, 10));
  if ( lpc == 0 && errno )
    throw smsc::util::Exception("SccpSubsystem::processRouteInfoToMTP3::: invalid value for parameter config.translation-table.entry.lpc");

  translation_table->addTranslationEntry(gt, routeId, true);
  addMTP3Policy(routeId, lpc, dpcsParam, trafficMode);
}

void
SccpSubsystem::initializeConnectAcceptor(const std::string& local_host,
                                         in_port_t local_port)
{
  snprintf(_acceptorName, sizeof(_acceptorName), "libSuaAcptr_%s.%d", local_host.c_str(), local_port);

  io_dispatcher::ConnectMgr::getInstance().addConnectAcceptor(_acceptorName,
                                                              new sap::LibSccpConnectAcceptor(_acceptorName, local_host, local_port));

  smsc_log_info(_logger, "SccpSubsystem::initialize::: ConnectAcceptor with name [%s] was initialized", _acceptorName);
}

std::string
SccpSubsystem::makeAddressFamilyPrefix(const std::string& gt_mask_value) const
{
  std::string gtAddressFamilyPrefix;

  if ( gt_mask_value[0] == '+' )
    gtAddressFamilyPrefix = ".4.0.1.4";
  else if ( gt_mask_value[0] >= 0x30 && gt_mask_value[0] <= 0x39 )
    gtAddressFamilyPrefix = ".4.0.1.0";
  else {
    std::string::size_type idx = gt_mask_value.rfind('.');
    if ( idx != std::string::npos )
      gtAddressFamilyPrefix = gt_mask_value.substr(0, idx);
    else
      throw smsc::util::Exception("SccpSubsystem::makeAddressFamilyPrefix::: can't determine address family prefix for gtMaskValue=[%s]", gt_mask_value.c_str());
  }

  return gtAddressFamilyPrefix;
}

void
SccpSubsystem::addSccpUserPolicy(const std::string& route_id,
                                 utilx::runtime_cfg::CompositeParameter* sccp_users_param,
                                 const std::string& traffic_mode)
{
  SCCPUserSelectPolicyRefPtr sccpUserSelectPolicyRefPtr =
      PolicyRegistry<SCCPUserSelectPolicy>::getInstance().lookup(route_id);
  if ( sccpUserSelectPolicyRefPtr.Get() )
    throw smsc::util::Exception("SccpSubsystem::addSccpUserPolicy::: SCCPUserSelectPolicy for route_id='%s' has already registered",
                                route_id.c_str());

  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::Parameter>
    userIdIterator = sccp_users_param->getIterator<utilx::runtime_cfg::Parameter>("userid");
  if ( !userIdIterator.hasElement() )
    throw smsc::util::Exception("SccpSubsystem::addSccpUserPolicy::: parameter config.translation-table.entry.sccp_users.userid is missed");

  if ( traffic_mode == "loadshare" ) {
    SCCPUserRoundRobinPolicy* sccpUserSelectPolicy = new SCCPUserRoundRobinPolicy();

    while (userIdIterator.hasElement()) {
      utilx::runtime_cfg::Parameter* nextParameter = userIdIterator.getCurrentElement();
      smsc_log_debug(_logger, "SccpSubsystem::addSccpUserPolicy::: add appId='%s' to SCCPUserRoundRobinPolicy for route_id='%s'",
                     nextParameter->getValue().c_str(), route_id.c_str());
      sccpUserSelectPolicy->addValue(nextParameter->getValue());
      userIdIterator.next();
    }

    sccpUserSelectPolicyRefPtr = sccpUserSelectPolicy;
  } else if ( traffic_mode == "dedicated" ) {
    SCCPUserDedicatedPolicy* sccpUserSelectPolicy = new SCCPUserDedicatedPolicy();
    utilx::runtime_cfg::Parameter* nextParameter = userIdIterator.getCurrentElement();
    smsc_log_debug(_logger, "SccpSubsystem::addSccpUserPolicy::: set appId='%s' for SCCPUserDedicatedPolicy for route_id='%s'",
                   nextParameter->getValue().c_str(), route_id.c_str());
    sccpUserSelectPolicy->setValue(nextParameter->getValue());
    userIdIterator.next();
    if ( userIdIterator.hasElement() )
      throw smsc::util::Exception("SccpSubsystem::addSccpUserPolicy::: wrong configuration - for 'dedicated' policy there are more than one value for parameter config.translation-table.entry.sccp_users.userid");

    sccpUserSelectPolicyRefPtr = sccpUserSelectPolicy;
  } else
    throw smsc::util::Exception("SccpSubsystem::addSccpUserPolicy::: invalid traffic-mode parameter value='%s'", traffic_mode.c_str());

  PolicyRegistry<SCCPUserSelectPolicy>::getInstance().insert(sccpUserSelectPolicyRefPtr, route_id);
}

void
SccpSubsystem::addMTP3Policy(const std::string& route_id,
                             common::point_code_t lpc,
                             utilx::runtime_cfg::CompositeParameter* dpcs_param,
                             const std::string& traffic_mode)
{
  MTP3SapSelectPolicyRefPtr mtp3SapPolicyRefPtr =
      PolicyRegistry<MTP3SapSelectPolicy>::getInstance().lookup(route_id);
  if ( mtp3SapPolicyRefPtr.Get() )
    throw smsc::util::Exception("SccpSubsystem::addMTP3Policy::: MTP3SapSelectPolicy for route_id='%s' has already registered",
                                route_id.c_str());

  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::Parameter>
    pcIterator = dpcs_param->getIterator<utilx::runtime_cfg::Parameter>("pc");
  if ( !pcIterator.hasElement() )
    throw smsc::util::Exception("SccpSubsystem::addMTP3Policy::: parameter config.translation-table.entry.dpcs.pc is missed");

  if ( traffic_mode == "loadshare" ) {
    MTP3SapRoundRobinPolicy* mtp3SapSelectPolicy = new MTP3SapRoundRobinPolicy();
    while ( pcIterator.hasElement() ) {
      common::point_code_t dpc = static_cast<common::point_code_t>(utilx::strtol(pcIterator.getCurrentElement()->getValue().c_str(), NULL, 10));
      if ( dpc == 0 && errno )
        throw smsc::util::Exception("SccpSubsystem::addMTP3Policy::: invalid value for parameter config.translation-table.dpcs.dpc");

      mtp3SapSelectPolicy->addValue(MTP3SapInfo(lpc, dpc));
      smsc_log_debug(_logger, "SccpSubsystem::addMTP3Policy::: add pair lpc=%u/dpc=%u to MTP3SapRoundRobinPolicy for route_id='%s'",
                     lpc, dpc, route_id.c_str());
      pcIterator.next();
    }

    mtp3SapPolicyRefPtr = mtp3SapSelectPolicy;
    smsc_log_debug(_logger, "SccpSubsystem::addMTP3Policy::: add MTP3SapRoundRobinPolicy to PolicyRegistry<MTP3SapSelectPolicy> for route_id='%s'",
                   route_id.c_str());
  } else if ( traffic_mode == "dedicated" ) {
    MTP3SapDedicatedPolicy* mtp3SapSelectPolicy = new MTP3SapDedicatedPolicy();
    utilx::runtime_cfg::Parameter* nextParameter = pcIterator.getCurrentElement();
    common::point_code_t dpc = static_cast<common::point_code_t>(utilx::strtol(nextParameter->getValue().c_str(), NULL, 10));
    if ( dpc == 0 && errno )
      throw smsc::util::Exception("SccpSubsystem::addMTP3Policy::: invalid value='%s' for parameter config.translation-table.dpcs.dpc",
                                  nextParameter->getValue().c_str());

    smsc_log_debug(_logger, "SccpSubsystem::addMTP3Policy::: set pair lpc=%u/dpc=%u for MTP3DedicatedPolicy for route_id='%s'",
                   lpc, dpc, route_id.c_str());
    mtp3SapSelectPolicy->setValue(MTP3SapInfo(lpc, dpc));
    pcIterator.next();
    if ( pcIterator.hasElement() )
      throw smsc::util::Exception("SccpSubsystem::addMTP3Policy::: wrong configuration - for 'dedicated' policy there are more than one value for parameter config.translation-table.entry.dpcs.pc");

    mtp3SapPolicyRefPtr = mtp3SapSelectPolicy;
    smsc_log_debug(_logger, "SccpSubsystem::addMTP3Policy::: add MTP3SapDedicatedPolicy to PolicyRegistry<MTP3SapSelectPolicy> for route_id='%s'",
                   route_id.c_str());
  } else
    throw smsc::util::Exception("SccpSubsystem::addMTP3Policy::: invalid traffic-mode parameter value='%s'", traffic_mode.c_str());

  PolicyRegistry<MTP3SapSelectPolicy>::getInstance().insert(mtp3SapPolicyRefPtr, route_id);
}

}}}}
