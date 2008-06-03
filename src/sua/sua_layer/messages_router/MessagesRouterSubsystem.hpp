#ifndef __SUA_SUALAYER_MESSAGESROUTER_MESSAGESROUTERSUBSYSTEM_HPP__
# define __SUA_SUALAYER_MESSAGESROUTER_MESSAGESROUTERSUBSYSTEM_HPP__ 1

# include <string>
# include <sua/utilx/Subsystem.hpp>
# include <sua/utilx/Singleton.hpp>
# include <util/config/ConfigView.h>
# include <netinet/in.h>
# include <sua/sua_layer/io_dispatcher/LinkSet.hpp>

namespace messages_router {

class MessagesRouterSubsystem : public utilx::Subsystem,
                                public utilx::Singleton<MessagesRouterSubsystem*> {
public:
  MessagesRouterSubsystem();
  virtual void start();
  virtual void stop();
  virtual void initialize(runtime_cfg::RuntimeConfig& rconfig);
  virtual void waitForCompletion();
  virtual const std::string& getName() const;
private:
  void initializeLinkSets(runtime_cfg::CompositeParameter& routingKeysParameter);

  bool processApplicationLinkSets(const runtime_cfg::CompositeParameter* routingEntryCompositeParameter,
                                  const communication::LinkId& linkSetId,
                                  io_dispatcher::LinkSet::linkset_mode_t linkSetMode);

  bool processSgpLinkSets(const runtime_cfg::CompositeParameter* routingEntryCompositeParameter,
                          const communication::LinkId& linkSetId,
                          io_dispatcher::LinkSet::linkset_mode_t linkSetMode);

  void fillUpRouteTable(const runtime_cfg::CompositeParameter* routingEntryCompositeParameter,
                        const communication::LinkId& linkSetId);

  io_dispatcher::LinkSet::linkset_mode_t convertStringToTrafficModeValue(const std::string& trafficMode,
                                                                         const std::string& where);

  std::string makeAddressFamilyPrefix(unsigned int gti,
                                      const std::string& gtMaskValue) const;

  std::string _name;
  smsc::logger::Logger* _logger;
};

}

#endif
