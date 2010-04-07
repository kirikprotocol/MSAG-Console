#ifndef __EYELINE_SS7NA_SUAGW_MESSAGESROUTER_MESSAGESROUTERSUBSYSTEM_HPP__
# define __EYELINE_SS7NA_SUAGW_MESSAGESROUTER_MESSAGESROUTERSUBSYSTEM_HPP__

# include <string>

# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"
# include "eyeline/utilx/runtime_cfg/ParameterObserver.hpp"

# include "eyeline/ss7na/common/io_dispatcher/LinkSet.hpp"
# include "eyeline/ss7na/common/ApplicationSubsystem.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace messages_router {

class MessagesRouterSubsystem : public common::ApplicationSubsystem,
                                public utilx::runtime_cfg::ParameterObserver,
                                public utilx::Singleton<MessagesRouterSubsystem*> {
public:
  MessagesRouterSubsystem();

  virtual void initialize(utilx::runtime_cfg::RuntimeConfig& rconfig);

  using ParameterObserver::addParameterEventHandler;
  using ParameterObserver::changeParameterEventHandler;
  using ParameterObserver::removeParameterEventHandler;

  virtual void changeParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                                           const utilx::runtime_cfg::Parameter& modified_parameter);

  virtual void addParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                                        utilx::runtime_cfg::Parameter* added_parameter);

  virtual utilx::runtime_cfg::CompositeParameter*
  addParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                           utilx::runtime_cfg::CompositeParameter* added_parameter);

  virtual void addParameterEventHandler(utilx::runtime_cfg::CompositeParameter* context,
                                        utilx::runtime_cfg::Parameter* added_parameter);

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

  void applyParametersChange(const common::LinkId& linkset_id,
                             common::io_dispatcher::LinkSet::linkset_mode_t linkset_mode,
                             utilx::runtime_cfg::CompositeParameter* routing_entry_composite_parameter);

  template<class LOOKED_FOR_PARAMETER_TYPE>
  bool checkParameterValueIsPresentInConfig(const std::string& parent_param_name,
                                            const std::string& interested_param_name,
                                            const std::string& value_to_compare);

  void addRouteEntry(utilx::runtime_cfg::CompositeParameter* route_description_composite_parameter,
                     const common::LinkId& linkset_id);

  class UncommitedRouteEntries {
  public:
    void addUncommitedRoute(const common::LinkId& linkset_id,
                            utilx::runtime_cfg::CompositeParameter* route_entry);

    bool fetchNextUncommitedRoute(common::LinkId* linkset_id,
                                  utilx::runtime_cfg::CompositeParameter** route_entry);
  private:
    typedef std::map<common::LinkId, utilx::runtime_cfg::CompositeParameter*> route_entry_cache_t;
    route_entry_cache_t _uncommitedRouteModificactionRequests;
  };

  UncommitedRouteEntries _uncommitedRoutesCache;
};

# include "MessagesRouterSubsystem_impl.hpp"

}}}}

#endif
