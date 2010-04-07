#include <utility>
#include "util/Exception.hpp"
#include "LinkSetsRegistry.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace sig {

void
LinkSetsRegistry::registerLinkSet(const common::LinkId& linkset_id)
{
  _registeredLinkSets.insert(linkset_id);
}

void
LinkSetsRegistry::registerLinkInLinkSet(const common::LinkId& linkset_id,
                                        const common::LinkId& link_id)
{
  if ( _registeredLinkSets.find(linkset_id) != _registeredLinkSets.end() ) {
    link_to_linksets_t::iterator iter = _link2linkSetAssociation.find(link_id);
    if ( iter == _link2linkSetAssociation.end() ) {
      std::pair<link_to_linksets_t::iterator, bool> insResult =_link2linkSetAssociation.insert(std::make_pair(link_id, new linksets_t()));
      iter = insResult.first;
    }
    iter->second->insert(linkset_id);
  } else
    throw smsc::util::Exception("LinkSetsRegistry::registerLinkInLinkSet::: linkSetId=[%s] wasn't registered",
                                linkset_id.getValue().c_str());
}

std::set<common::LinkId>
LinkSetsRegistry::getLinkSetsByLink(const common::LinkId& link_id)
{
  link_to_linksets_t::iterator iter = _link2linkSetAssociation.find(link_id);
  if ( iter != _link2linkSetAssociation.end() )
    return *iter->second;
  else
    throw smsc::util::Exception("LinkSetsRegistry::getLinkSetsByLink::: linkId=[%s] wasn't registered", link_id.getValue().c_str());
}

bool
LinkSetsRegistry::checkLinkIsMemberOfAnyLinkset(const common::LinkId& link_id)
{
  link_to_linksets_t::iterator iter = _link2linkSetAssociation.find(link_id);
  if ( iter != _link2linkSetAssociation.end() )
    return true;
  else
    return false;
}

}}}}
