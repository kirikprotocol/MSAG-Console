#ifndef __EYELINE_SS7NA_M3UAGW_MTP3_M3UASTACK_MTP3SUBSYSTEM_HPP__
# define __EYELINE_SS7NA_M3UAGW_MTP3_M3UASTACK_MTP3SUBSYSTEM_HPP__

# include <set>
# include <vector>
# include <string>
# include <netinet/in.h>

# include "core/synchronization/EventMonitor.hpp"

# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"

# include "eyeline/ss7na/common/LinkId.hpp"
# include "eyeline/ss7na/common/ApplicationSubsystem.hpp"
# include "eyeline/ss7na/common/types.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {

class MTP3Subsystem : public common::ApplicationSubsystem,
                      public utilx::Singleton<MTP3Subsystem*> {
public:
  virtual void stop();
  virtual void initialize(utilx::runtime_cfg::RuntimeConfig& rconfig);
  virtual void waitForCompletion();

  virtual void notifyLinkShutdownCompletion();

  void activateLinkToSGP(const common::LinkId& link_id,
                         const std::vector<std::string>& r_addr,
                         in_port_t r_port,
                         const std::vector<std::string>& l_addr,
                         in_port_t l_port);

protected:
  MTP3Subsystem();
  friend class utilx::Singleton<MTP3Subsystem*>;

private:
  std::vector<common::LinkId> _sgpLinkIds;
  smsc::core::synchronization::EventMonitor _allLinksShutdownMonitor;
  volatile int _establishedLinks;

  void extractAddressParameters(std::vector<std::string>* addrs,
                                utilx::runtime_cfg::CompositeParameter* next_parameter,
                                const std::string& param_name);

  void configurePoints(utilx::runtime_cfg::RuntimeConfig& rconfig);
  void fillInRoutingTable(utilx::runtime_cfg::RuntimeConfig& rconfig);
  void activateLinksToSGP(utilx::runtime_cfg::RuntimeConfig& rconfig);
  void addMtp3Route(common::point_code_t lpc, common::point_code_t dpc,
                    const common::LinkId& sgp_link_id);
};

}}}}

#endif
