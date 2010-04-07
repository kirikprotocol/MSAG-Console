#ifndef __EYELINE_SS7NA_COMMON_SIG_LINKSETSREGISTRY_HPP__
# define __EYELINE_SS7NA_COMMON_SIG_LINKSETSREGISTRY_HPP__

# include <map>
# include <set>
# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/ss7na/common/LinkId.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace sig {

class LinkSetsRegistry : public utilx::Singleton<LinkSetsRegistry> {
public:
  void registerLinkSet(const common::LinkId& linkset_id);
  void registerLinkInLinkSet(const common::LinkId& linkset_id,
                             const common::LinkId& link_id);
  std::set<common::LinkId> getLinkSetsByLink(const common::LinkId& link_id);
  bool checkLinkIsMemberOfAnyLinkset(const common::LinkId& link_id_to_sgp);

private:
  typedef std::set<common::LinkId> linksets_t;
  linksets_t _registeredLinkSets;
  typedef std::map<common::LinkId, linksets_t*> link_to_linksets_t;
  link_to_linksets_t _link2linkSetAssociation;
};

}}}}

#endif
