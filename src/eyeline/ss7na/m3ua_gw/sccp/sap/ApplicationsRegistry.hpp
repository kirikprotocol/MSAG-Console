#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_SAP_APPLICATIONSREGISTRY_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_SAP_APPLICATIONSREGISTRY_HPP__

# include <string>

# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/ss7na/common/KeysRegistry.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace sap {

class ApplicationsRegistry : public utilx::Singleton<ApplicationsRegistry>,
                             public common::KeysRegistry<std::string /*app_id*/> {
private:
  ApplicationsRegistry() {}
  friend class utilx::Singleton<ApplicationsRegistry>;
};

}}}}}

#endif
