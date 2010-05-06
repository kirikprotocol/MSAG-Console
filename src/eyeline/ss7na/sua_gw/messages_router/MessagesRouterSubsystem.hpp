#ifndef __EYELINE_SS7NA_SUAGW_MESSAGESROUTER_MESSAGESROUTERSUBSYSTEM_HPP__
# define __EYELINE_SS7NA_SUAGW_MESSAGESROUTER_MESSAGESROUTERSUBSYSTEM_HPP__

# include <string>

# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"

# include "eyeline/ss7na/common/io_dispatcher/LinkSet.hpp"
# include "eyeline/ss7na/common/ApplicationSubsystem.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace messages_router {

class MessagesRouterSubsystem : public common::ApplicationSubsystem,
                                public utilx::Singleton<MessagesRouterSubsystem*> {
public:
  MessagesRouterSubsystem();

  virtual void initialize(utilx::runtime_cfg::RuntimeConfig& rconfig);

private:
  void initializeLinkSets(utilx::runtime_cfg::CompositeParameter& routing_keys_parameter);

  bool processApplicationLinkSets(utilx::runtime_cfg::CompositeParameter* routing_entry_composite_parameter,
                                  const common::LinkId& linkset_id,
                                  common::io_dispatcher::LinkSet::linkset_mode_t linkset_mode);

  bool processSgpLinkSets(utilx::runtime_cfg::CompositeParameter* routing_entry_composite_parameter,
                          const common::LinkId& linkset_id,
                          common::io_dispatcher::LinkSet::linkset_mode_t linkset_mode);

  void fillUpRouteTable(utilx::runtime_cfg::CompositeParameter* routing_entry_composite_parameter,
                        const common::LinkId& linkset_id);

  common::io_dispatcher::LinkSet::linkset_mode_t
  convertStringToTrafficModeValue(const std::string& traffic_mode,
                                  const std::string& where);

  std::string makeAddressFamilyPrefix(unsigned int gti,
                                      const std::string& gt_mask_value) const;

  template<class LOOKED_FOR_PARAMETER_TYPE>
  bool checkParameterValueIsPresentInConfig(const std::string& parent_param_name,
                                            const std::string& interested_param_name,
                                            const std::string& value_to_compare);

  void addRouteEntry(utilx::runtime_cfg::CompositeParameter* route_description_composite_parameter,
                     const common::LinkId& linkset_id);

};

# include "MessagesRouterSubsystem_impl.hpp"

}}}}

#endif
