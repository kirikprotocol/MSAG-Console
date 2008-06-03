#include <utility>
#include <util/Exception.hpp>
#include "LinkSetsRegistry.hpp"

namespace sua_stack {

static bool
initialize()
{
  LinkSetsRegistry::init();
  return true;
}

static bool _initialized = initialize();

void
LinkSetsRegistry::registerLinkSet(const communication::LinkId& linkSetId)
{
  _registeredLinkSets.insert(linkSetId);
}

void
LinkSetsRegistry::registerLinkInLinkSet(const communication::LinkId& linkSetId,
                                        const communication::LinkId& linkId)
{
  if ( _registeredLinkSets.find(linkSetId) != _registeredLinkSets.end() ) {
    link_to_linksets_t::iterator iter = _link2linkSetAssociation.find(linkId);
    if ( iter == _link2linkSetAssociation.end() ) {
      std::pair<link_to_linksets_t::iterator, bool> insResult =_link2linkSetAssociation.insert(std::make_pair(linkId, new linksets_t()));
      iter = insResult.first;
    }
    iter->second->insert(linkSetId);
  } else
    throw smsc::util::Exception("LinkSetsRegistry::registerLinkInLinkSet::: linkSetId=[%s] wasn't registered", linkSetId.getValue().c_str());
}

std::set<communication::LinkId>
LinkSetsRegistry::getLinkSetsByLink(const communication::LinkId& linkId)
{
  link_to_linksets_t::iterator iter = _link2linkSetAssociation.find(linkId);
  if ( iter != _link2linkSetAssociation.end() )
    return *iter->second;
  else
    throw smsc::util::Exception("LinkSetsRegistry::getLinkSetsByLink::: linkId=[%s] wasn't registered", linkId.getValue().c_str());
}

bool
LinkSetsRegistry::checkLinkIsMemberOfAnyLinkset(const communication::LinkId& linkId)
{
  link_to_linksets_t::iterator iter = _link2linkSetAssociation.find(linkId);
  if ( iter != _link2linkSetAssociation.end() )
    return true;
  else
    return false;
}

}
