#include <utility>
#include <util/Exception.hpp>
#include <core/synchronization/MutexGuard.hpp>

#include "LinkSetInfoRegistry.hpp"

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace sua_user_communication {

LinkSetInfoRegistry::LinkSetInfoRegistry()
  : _logger(smsc::logger::Logger::getInstance("sua_usr_cm")) {}

void
LinkSetInfoRegistry::addAssociation(const communication::LinkId& linkSetId, const std::string& appId)
{
  smsc::core::synchronization::MutexGuard synchonize(_lock);

  registry_t::iterator iter = _registry.find(linkSetId);
  if ( iter != _registry.end() )
    iter->second->insert(appId);
  else {
    std::set<std::string>* appSet = new std::set<std::string>();
    appSet->insert(appId);
    _registry.insert(std::make_pair(linkSetId, appSet));
  }
  smsc_log_info(_logger, "LinkSetInfoRegistry::addAssociation::: added association for appId=[%s] to linkSet=[%s]", appId.c_str(), linkSetId.getValue().c_str());
}

bool
LinkSetInfoRegistry::getLinkSet(const std::string& appId, communication::LinkId* linkSetId) const
{
  smsc::core::synchronization::MutexGuard synchonize(_lock);

  for(registry_t::const_iterator iter = _registry.begin(), end_iter = _registry.end(); iter != end_iter; ++iter) {
    if ( iter->second->find(appId) != iter->second->end() ) {
      *linkSetId = iter->first;
      return true;
    }
  }

  smsc_log_info(_logger, "LinkSetInfoRegistry::getLinkSet::: there isn't registered linkSetId for appId=[%s]", appId.c_str());
  return false;
}

void
LinkSetInfoRegistry::removeAssociation(const communication::LinkId& linkSetId, const std::string& appId)
{
  smsc::core::synchronization::MutexGuard synchonize(_lock);

  registry_t::iterator iter = _registry.find(linkSetId);
  if ( iter != _registry.end() )
    if ( iter->second->erase(appId) > 0 && iter->second->empty() ) {
      smsc_log_info(_logger, "LinkSetInfoRegistry::removeAssociation::: removed association between appId=[%s] and linkSet=[%s]", appId.c_str(), iter->first.getValue().c_str());
      delete iter->second;
      _registry.erase(iter);
      return;
    }
}

std::set<communication::LinkId>
LinkSetInfoRegistry::getLinkSetIds() const
{
  smsc::core::synchronization::MutexGuard synchonize(_lock);

  std::set<communication::LinkId> result;
  for(registry_t::const_iterator iter = _registry.begin(), end_iter = _registry.end();
      iter != end_iter; ++iter)
    result.insert(iter->first);

  return result;
}

bool
LinkSetInfoRegistry::getAppId(const communication::LinkId& linkSetId, std::string* appId)
{
  registry_t::iterator iter = _registry.find(linkSetId);
  if ( iter != _registry.end() ) {
    *appId = *(iter->second->begin());
    return true;
  } else
    return false;
}

}}}}
