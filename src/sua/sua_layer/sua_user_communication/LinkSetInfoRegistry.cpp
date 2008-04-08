#include "LinkSetInfoRegistry.hpp"
#include <util/Exception.hpp>
#include <utility>

namespace sua_user_communication {

LinkSetInfoRegistry*
utilx::Singleton<LinkSetInfoRegistry>::_instance;

LinkSetInfoRegistry::LinkSetInfoRegistry()
  : _logger(smsc::logger::Logger::getInstance("sua_usr_cm")) {}

void
LinkSetInfoRegistry::addAssociation(const communication::LinkId& linkSetId, const std::string& appId) {
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

const communication::LinkId&
LinkSetInfoRegistry::getLinkSet(const std::string& appId) {
  for(registry_t::const_iterator iter = _registry.begin(), end_iter = _registry.end(); iter != end_iter; ++iter) {
    if ( iter->second->find(appId) != iter->second->end() )
      return iter->first;
  }
  throw smsc::util::Exception("LinkSetInfoRegistry::getLinkSet::: there isn't registered linkSetId for appId=[%s]", appId.c_str());
}

void
LinkSetInfoRegistry::removeAssociation(const communication::LinkId& linkSetId, const std::string& appId)
{
  registry_t::iterator iter = _registry.find(linkSetId);
  if ( iter != _registry.end() )
    if ( iter->second->erase(appId) > 0 && iter->second->empty() ) {
      smsc_log_info(_logger, "LinkSetInfoRegistry::removeAssociation::: removed association between appId=[%s] and linkSet=[%s]", appId.c_str(), iter->first.getValue().c_str());
      delete iter->second;
      _registry.erase(iter);
      return;
    }
}

}
