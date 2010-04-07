#ifndef __EYELINE_SS7NA_M3UAGW_RUNTIMECFG_RUNTIMECONFIG_HPP__
# define __EYELINE_SS7NA_M3UAGW_RUNTIMECFG_RUNTIMECONFIG_HPP__

# include <string>
# include <map>
# include "logger/Logger.h"
# include "util/config/ConfigView.h"
# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace runtime_cfg {

class RuntimeConfig : public utilx::Singleton<RuntimeConfig>,
                      public utilx::runtime_cfg::RuntimeConfig {
public:
  RuntimeConfig();

  using utilx::runtime_cfg::RuntimeConfig::initialize;

  virtual void initialize(smsc::util::config::ConfigView& m3uagw_cfg);

private:
  void processSccpUsersSection(smsc::util::config::ConfigView& m3ua_cfg,
                               utilx::runtime_cfg::CompositeParameter* m3ua_cfg_param);

  void processSgpLinksSection(smsc::util::config::ConfigView& m3ua_cfg,
                              utilx::runtime_cfg::CompositeParameter* m3ua_cfg_param);

  void processMtp3RoutingTableSection(smsc::util::config::ConfigView& m3ua_cfg,
                                      utilx::runtime_cfg::CompositeParameter* m3ua_cfg_param);

  void processTranslationTableSection(smsc::util::config::ConfigView& m3ua_cfg,
                                      utilx::runtime_cfg::CompositeParameter* m3ua_cfg_param);

  void processPointsDefinitionSection(smsc::util::config::ConfigView& m3ua_cfg,
                                      utilx::runtime_cfg::CompositeParameter* m3ua_cfg_param);

  smsc::logger::Logger* _logger;
};

}}}}

#endif
