#include <algorithm>
#include <utility>
#include "ActiveAppsRegistry.hpp"
#include "util/Exception.hpp"
#include "eyeline/utilx/Exception.hpp"
#include "eyeline/ss7na/common/Exception.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace sccp_sap {

void
ActiveAppsRegistry::insert(const uint8_t* ssn_begin, const uint8_t* ssn_end,
                          const LinkId& link_id_to_application,
                          const std::string& app_id)
{
  smsc::core::synchronization::MutexGuard synchronize(_lock);

  if ( _activeApps.find(app_id) != _activeApps.end() )
    throw utilx::DuplicatedRegistryKeyException("ActiveAppsRegistry::insert::: application with id=%s has been already registered",
                                                app_id.c_str());
  if ( _activeLinks.find(link_id_to_application) != _activeLinks.end() )
    throw smsc::util::Exception("ActiveAppsRegistry::insert::: link with id=%s has been already registered",
                                link_id_to_application.getValue().c_str());

  std::pair<active_apps_t::iterator, bool> ins_res =
      _activeApps.insert(std::make_pair(app_id,
                                        app_info(link_id_to_application, ssn_begin, ssn_end)));
  if ( !ins_res.second )
    throw smsc::util::Exception("ActiveAppsRegistry::insert::: insert into _activeApps failed for appId=%s/link_id=%s",
                                app_id.c_str(), link_id_to_application.getValue().c_str());

  std::pair<active_links_t::iterator, bool> activeLnk_ins_res =
      _activeLinks.insert(std::make_pair(link_id_to_application, ins_res.first));
  if ( !activeLnk_ins_res.second )
    throw smsc::util::Exception("ActiveAppsRegistry::insert::: insert into _activeLinks failed for appId=%s/link_id=%s",
                                app_id.c_str(), link_id_to_application.getValue().c_str());

  while(ssn_begin < ssn_end) {
    uint8_t ssnValue = *ssn_begin++;
    if ( !_activeSsns[ssnValue] )
      _activeSsns[ssnValue] = new connected_apps();
    _activeSsns[ssnValue]->activeApplications.push_back(link_id_to_application);
  }
}

void
ActiveAppsRegistry::remove(const LinkId& link_id_to_application)
{
  if ( link_id_to_application.isEmpty() )
    return;

  smsc::core::synchronization::MutexGuard synchronize(_lock);

  active_links_t::iterator iter = _activeLinks.find(link_id_to_application);
  if ( iter == _activeLinks.end() )
    return;

  smsc_log_info(_logger, "ActiveAppsRegistry::remove::: unregister application for link with id='%s'",
                link_id_to_application.getValue().c_str());

  const app_info& appInfo = iter->second->second;
  for(unsigned i=0; i < appInfo.maxSsnNum; ++i) {
    uint8_t ssnValue = appInfo.serviceableSsns[i];
    if ( ssnValue ) {
      connected_apps* connectedApps = _activeSsns[ssnValue];
      if ( connectedApps ) {
        std::list<LinkId>::iterator activeAppIter =
            std::find(connectedApps->activeApplications.begin(), connectedApps->activeApplications.end(),
                      link_id_to_application);
        if ( activeAppIter != connectedApps->activeApplications.end() )
          connectedApps->activeApplications.erase(activeAppIter);
      }
    }
  }
  _activeApps.erase(iter->second);
  _activeLinks.erase(iter);
}

LinkId
ActiveAppsRegistry::getLink(uint8_t ssn) const
{
  smsc::core::synchronization::MutexGuard synchronize(_lock);

  if ( !_activeSsns[ssn] )
    throw RouteNotFound(SUBSYSTEM_FAILURE, "ActiveAppsRegistry::getLink::: ssn=%d is not serviceable",
                        ssn);
  if ( _activeSsns[ssn]->activeApplications.empty() )
    throw RouteNotFound(SUBSYSTEM_FAILURE, "ActiveAppsRegistry::getLink::: there isn't active application for ssn=%d",
                        ssn);

  return *_activeSsns[ssn]->activeApplications.begin();
}

LinkId
ActiveAppsRegistry::getLink(const std::string& app_id, uint8_t ssn) const
{
  smsc::core::synchronization::MutexGuard synchronize(_lock);

  active_apps_t::const_iterator iter = _activeApps.find(app_id);
  if ( iter == _activeApps.end() )
    throw RouteNotFound(SUBSYSTEM_FAILURE, "ActiveAppsRegistry::getLink::: there isn't active application with id=%s",
                        app_id.c_str());
  if ( iter->second.serviceableSsns[ssn] )
    return iter->second.linkId;
  throw RouteNotFound(SUBSYSTEM_FAILURE, "ActiveAppsRegistry::getLink::: there isn't active application with id=%s and ssn=%d",
                      app_id.c_str(), ssn);
}

LinkId
ActiveAppsRegistry::getLink(const std::string& app_id) const
{
  smsc::core::synchronization::MutexGuard synchronize(_lock);

  active_apps_t::const_iterator iter = _activeApps.find(app_id);
  if ( iter == _activeApps.end() )
    throw RouteNotFound(SUBSYSTEM_FAILURE, "ActiveAppsRegistry::getLink::: there isn't active application with id=%s",
                        app_id.c_str());

  return iter->second.linkId;
}

void
ActiveAppsRegistry::getAllActiveAppLinks(std::list<LinkId>* linkid_list) const
{
  smsc::core::synchronization::MutexGuard synchronize(_lock);
  for(active_links_t::const_iterator iter = _activeLinks.begin(), end_iter = _activeLinks.end();
      iter != end_iter; ++iter)
    linkid_list->push_back(iter->first);
}

}}}}
