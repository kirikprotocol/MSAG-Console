#ifndef __EYELINE_SUA_SUALAYER_MESSAGESROUTER_MESSAGESROUTERSUBSYSTEM_HPP__
# define __EYELINE_SUA_SUALAYER_MESSAGESROUTER_MESSAGESROUTERSUBSYSTEM_HPP__

# include <string>

# include <eyeline/utilx/Singleton.hpp>
# include <eyeline/utilx/runtime_cfg/RuntimeConfig.hpp>
# include <eyeline/utilx/runtime_cfg/ParameterObserver.hpp>

# include <eyeline/sua/sua_layer/ApplicationSubsystem.hpp>
# include <eyeline/sua/sua_layer/io_dispatcher/LinkSet.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace messages_router {

class MessagesRouterSubsystem : public sua_layer::ApplicationSubsystem,
                                public utilx::runtime_cfg::ParameterObserver,
                                public utilx::Singleton<MessagesRouterSubsystem*> {
public:
  MessagesRouterSubsystem();

  virtual void initialize(utilx::runtime_cfg::RuntimeConfig& rconfig);

  using ParameterObserver::addParameterEventHandler;
  using ParameterObserver::changeParameterEventHandler;
  using ParameterObserver::removeParameterEventHandler;

  virtual void changeParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                                           const utilx::runtime_cfg::Parameter& modifiedParameter);

  virtual void addParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                                        utilx::runtime_cfg::Parameter* addedParameter);

  virtual utilx::runtime_cfg::CompositeParameter*
  addParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                           utilx::runtime_cfg::CompositeParameter* addedParameter);

  virtual void addParameterEventHandler(utilx::runtime_cfg::CompositeParameter* context,
                                        utilx::runtime_cfg::Parameter* addedParameter);

private:
  void initializeLinkSets(utilx::runtime_cfg::CompositeParameter& routingKeysParameter);

  bool processApplicationLinkSets(utilx::runtime_cfg::CompositeParameter* routingEntryCompositeParameter,
                                  const communication::LinkId& linkSetId,
                                  io_dispatcher::LinkSet::linkset_mode_t linkSetMode);

  bool processSgpLinkSets(utilx::runtime_cfg::CompositeParameter* routingEntryCompositeParameter,
                          const communication::LinkId& linkSetId,
                          io_dispatcher::LinkSet::linkset_mode_t linkSetMode);

  void fillUpRouteTable(utilx::runtime_cfg::CompositeParameter* routingEntryCompositeParameter,
                        const communication::LinkId& linkSetId);

  io_dispatcher::LinkSet::linkset_mode_t convertStringToTrafficModeValue(const std::string& trafficMode,
                                                                         const std::string& where);

  std::string makeAddressFamilyPrefix(unsigned int gti,
                                      const std::string& gtMaskValue) const;

  void applyParametersChange(const communication::LinkId& linkSetId,
                             io_dispatcher::LinkSet::linkset_mode_t linkSetMode,
                             utilx::runtime_cfg::CompositeParameter* routingEntryCompositeParameter);

  template<class LOOKED_FOR_PARAMETER_TYPE>
  bool checkParameterValueIsPresentInConfig(const std::string& parentParamName,
                                            const std::string& interestedParamName,
                                            const std::string& valueToCompare);

  void addRouteEntry(utilx::runtime_cfg::CompositeParameter* routeDescriptionCompositeParameter,
                     const communication::LinkId& linkSetId);

  class UncommitedRouteEntries {
  public:
    void addUncommitedRoute(const communication::LinkId& linkSetId,
                            utilx::runtime_cfg::CompositeParameter* routeEntry);

    bool fetchNextUncommitedRoute(communication::LinkId* linkSetId,
                                  utilx::runtime_cfg::CompositeParameter** routeEntry);
  private:
    typedef std::map<communication::LinkId, utilx::runtime_cfg::CompositeParameter*> route_entry_cache_t;
    route_entry_cache_t _uncommitedRouteModificactionRequests;
  };

  UncommitedRouteEntries _uncommitedRoutesCache;
};

# include "MessagesRouterSubsystem_impl.hpp"

}}}}

#endif
