#include <utility>
#include "util/Exception.hpp"
#include "core/synchronization/MutexGuard.hpp"

#include "LinkSetInfoRegistry.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sccp_sap {

void
LinkSetInfoRegistry::addAssociation(const common::LinkId& linkset_id,
                                    const std::string& app_id)
{
  smsc::core::synchronization::MutexGuard synchonize(_lock);

  registry_t::iterator iter = _registry.find(linkset_id);
  if ( iter != _registry.end() )
    iter->second->insert(app_id);
  else {
    std::set<std::string>* appSet = new std::set<std::string>();
    appSet->insert(app_id);
    _registry.insert(std::make_pair(linkset_id, appSet));
  }
  smsc_log_info(_logger, "LinkSetInfoRegistry::addAssociation::: added association for appId=[%s] to linkSet=[%s]", app_id.c_str(), linkset_id.getValue().c_str());
}

bool
LinkSetInfoRegistry::getLinkSet(const std::string& app_id,
                                common::LinkId* linkset_id) const
{
  smsc::core::synchronization::MutexGuard synchonize(_lock);

  for(registry_t::const_iterator iter = _registry.begin(), end_iter = _registry.end(); iter != end_iter; ++iter) {
    if ( iter->second->find(app_id) != iter->second->end() ) {
      *linkset_id = iter->first;
      return true;
    }
  }

  smsc_log_info(_logger, "LinkSetInfoRegistry::getLinkSet::: there isn't registered linkSetId for appId=[%s]", app_id.c_str());
  return false;
}

void
LinkSetInfoRegistry::removeAssociation(const common::LinkId& linkset_id,
                                       const std::string& app_id)
{
  smsc::core::synchronization::MutexGuard synchonize(_lock);

  registry_t::iterator iter = _registry.find(linkset_id);
  if ( iter != _registry.end() )
    if ( iter->second->erase(app_id) > 0 && iter->second->empty() ) {
      smsc_log_info(_logger, "LinkSetInfoRegistry::removeAssociation::: removed association between appId=[%s] and linkSet=[%s]", app_id.c_str(), iter->first.getValue().c_str());
      delete iter->second;
      _registry.erase(iter);
      return;
    }
}

std::set<common::LinkId>
LinkSetInfoRegistry::getLinkSetIds() const
{
  smsc::core::synchronization::MutexGuard synchonize(_lock);

  std::set<common::LinkId> result;
  for(registry_t::const_iterator iter = _registry.begin(), end_iter = _registry.end();
      iter != end_iter; ++iter)
    result.insert(iter->first);

  return result;
}

bool
LinkSetInfoRegistry::getAppId(const common::LinkId& linkset_id,
                              std::string* app_id)
{
  registry_t::iterator iter = _registry.find(linkset_id);
  if ( iter != _registry.end() ) {
    *app_id = *(iter->second->begin());
    return true;
  } else
    return false;
}

}}}}
