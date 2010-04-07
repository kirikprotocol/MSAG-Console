#ifndef __EYELINE_SS7NA_M3UAGW_MTP3_SGPLINKIDSREGISTRY_HPP__
# define __EYELINE_SS7NA_M3UAGW_MTP3_SGPLINKIDSREGISTRY_HPP__

# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/ss7na/common/LinkId.hpp"
# include "eyeline/ss7na/common/KeysRegistry.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {

class SGPLinkIdsRegistry : public utilx::Singleton<SGPLinkIdsRegistry>,
                           public common::KeysRegistry<common::LinkId> {
private:
  SGPLinkIdsRegistry() {}
  friend class utilx::Singleton<SGPLinkIdsRegistry>;
};

}}}}

#endif
