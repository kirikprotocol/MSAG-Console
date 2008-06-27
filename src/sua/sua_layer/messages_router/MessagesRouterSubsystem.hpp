#ifndef __SUA_SUALAYER_MESSAGESROUTER_MESSAGESROUTERSUBSYSTEM_HPP__
# define __SUA_SUALAYER_MESSAGESROUTER_MESSAGESROUTERSUBSYSTEM_HPP__ 1

# include <string>

# include <sua/utilx/Singleton.hpp>
# include <sua/sua_layer/ApplicationSubsystem.hpp>
# include <sua/sua_layer/io_dispatcher/LinkSet.hpp>
# include <sua/sua_layer/runtime_cfg/RuntimeConfig.hpp>
# include <sua/sua_layer/runtime_cfg/ParameterObserver.hpp>

namespace messages_router {

class MessagesRouterSubsystem : public sua_layer::ApplicationSubsystem,
                                public runtime_cfg::ParameterObserver,
                                public utilx::Singleton<MessagesRouterSubsystem*> {
public:
  MessagesRouterSubsystem();

  virtual void initialize(runtime_cfg::RuntimeConfig& rconfig);

  using ParameterObserver::addParameterEventHandler;
  using ParameterObserver::changeParameterEventHandler;
  using ParameterObserver::removeParameterEventHandler;

  virtual void changeParameterEventHandler(const runtime_cfg::CompositeParameter& context,
                                           const runtime_cfg::Parameter& modifiedParameter);

  virtual void addParameterEventHandler(const runtime_cfg::CompositeParameter& context,
                                        runtime_cfg::Parameter* addedParameter);

  virtual runtime_cfg::CompositeParameter*
  addParameterEventHandler(const runtime_cfg::CompositeParameter& context,
                           runtime_cfg::CompositeParameter* addedParameter);

  virtual void addParameterEventHandler(runtime_cfg::CompositeParameter* context,
                                        runtime_cfg::Parameter* addedParameter);

private:
  void initializeLinkSets(runtime_cfg::CompositeParameter& routingKeysParameter);

  bool processApplicationLinkSets(runtime_cfg::CompositeParameter* routingEntryCompositeParameter,
                                  const communication::LinkId& linkSetId,
                                  io_dispatcher::LinkSet::linkset_mode_t linkSetMode);

  bool processSgpLinkSets(runtime_cfg::CompositeParameter* routingEntryCompositeParameter,
                          const communication::LinkId& linkSetId,
                          io_dispatcher::LinkSet::linkset_mode_t linkSetMode);

  void fillUpRouteTable(runtime_cfg::CompositeParameter* routingEntryCompositeParameter,
                        const communication::LinkId& linkSetId);

  io_dispatcher::LinkSet::linkset_mode_t convertStringToTrafficModeValue(const std::string& trafficMode,
                                                                         const std::string& where);

  std::string makeAddressFamilyPrefix(unsigned int gti,
                                      const std::string& gtMaskValue) const;

  void applyParametersChange(const communication::LinkId& linkSetId,
                             io_dispatcher::LinkSet::linkset_mode_t linkSetMode,
                             runtime_cfg::CompositeParameter* routingEntryCompositeParameter);

  template<class LOOKED_FOR_PARAMETER_TYPE>
  bool checkParameterValueIsPresentInConfig(const std::string& parentParamName,
                                            const std::string& interestedParamName,
                                            const std::string& valueToCompare);

  void addRouteEntry(runtime_cfg::CompositeParameter* routeDescriptionCompositeParameter,
                     const communication::LinkId& linkSetId);

  class UncommitedRouteEntries {
  public:
    void addUncommitedRoute(const communication::LinkId& linkSetId,
                            runtime_cfg::CompositeParameter* routeEntry);

    bool fetchNextUncommitedRoute(communication::LinkId* linkSetId,
                                  runtime_cfg::CompositeParameter** routeEntry);
  private:
    typedef std::map<communication::LinkId, runtime_cfg::CompositeParameter*> route_entry_cache_t;
    route_entry_cache_t _uncommitedRouteModificactionRequests;
  };

  UncommitedRouteEntries _uncommitedRoutesCache;
};

# include "MessagesRouterSubsystem_impl.hpp"

}

#endif
