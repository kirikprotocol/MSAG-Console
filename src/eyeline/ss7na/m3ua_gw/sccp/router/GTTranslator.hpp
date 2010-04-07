#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_ROUTER_GTTRANSLATOR_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_ROUTER_GTTRANSLATOR_HPP__

# include "logger/Logger.h"
# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/ss7na/m3ua_gw/types.hpp"

# include "eyeline/sccp/SCCPAddress.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace router {

struct GTTranslationResult {
  GTTranslationResult(const std::string& route_id, bool route_to_mtp3)
  : routeId(route_id), routeToMTP3(route_to_mtp3)
  {}
  GTTranslationResult(const eyeline::sccp::SCCPAddress& new_called_address,
                      const std::string& route_id, bool route_to_mtp3)
  : newCalledAddress(new_called_address), routeId(route_id), routeToMTP3(route_to_mtp3)
  {}
  eyeline::sccp::SCCPAddress newCalledAddress;
  std::string routeId; // e.g. MasterGT, ExKrasnoyarskGT, CLOUD
  bool routeToMTP3; // if true -> try find link by point code, else try find link by sccp user appId
};

class GTTranslator : public utilx::Singleton<GTTranslator> {
public:
  GTTranslationResult translate(const eyeline::sccp::SCCPAddress& address);

private:
  GTTranslator();
  friend class utilx::Singleton<GTTranslator>;

  GTTranslator(const GTTranslator& rhs);
  GTTranslator& operator=(const GTTranslator& rhs);
  smsc::logger::Logger* _logger;
};

}}}}}

#endif
