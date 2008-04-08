#ifndef __SUA_SUALAYER_SUASTACK_LINKSETSREGISTRY_HPP__
# define __SUA_SUALAYER_SUASTACK_LINKSETSREGISTRY_HPP__ 1

# include <map>
# include <set>
# include <sua/utilx/Singleton.hpp>
# include <sua/communication/LinkId.hpp>

namespace sua_stack {

class LinkSetsRegistry : public utilx::Singleton<LinkSetsRegistry> {
public:
  void registerLinkSet(const communication::LinkId& linkSetId);
  void registerLinkInLinkSet(const communication::LinkId& linkSetId, const communication::LinkId& linkId);
  std::set<communication::LinkId> getLinkSetsByLink(const communication::LinkId& linkId);
  bool checkLinkIsMemberOfAnyLinkset(const communication::LinkId& linkIdToSgp);
private:
  typedef std::set<communication::LinkId> linksets_t;
  linksets_t _registeredLinkSets;
  typedef std::map<communication::LinkId, linksets_t*> link_to_linksets_t;
  link_to_linksets_t _link2linkSetAssociation;
};

}

#endif
