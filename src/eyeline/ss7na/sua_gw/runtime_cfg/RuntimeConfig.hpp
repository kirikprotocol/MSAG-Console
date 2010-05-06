#ifndef __EYELINE_SS7NA_SUAGW_RUNTIMECFG_RUNTIMECONFIG_HPP__
# define __EYELINE_SS7NA_SUAGW_RUNTIMECFG_RUNTIMECONFIG_HPP__

# include <string>
# include <map>
# include "logger/Logger.h"
# include "util/config/ConfigView.h"
# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace runtime_cfg {

class RuntimeConfig : public utilx::Singleton<RuntimeConfig>,
                      public utilx::runtime_cfg::RuntimeConfig {
public:
  RuntimeConfig();

  using utilx::runtime_cfg::RuntimeConfig::initialize;

  virtual void initialize(smsc::util::config::ConfigView& xmlConfig,
                          const char* cfg_file);
private:
  void processRoutingKeysSection(smsc::util::config::ConfigView& suaLayerCfg,
                                 const char* rkSectionName, // e.g. "incoming-routing-keys"
                                 utilx::runtime_cfg::CompositeParameter* suaConfigCompositeParameter);
  void processSuaApplications(smsc::util::config::ConfigView& sua_layer_cfg,
                              utilx::runtime_cfg::CompositeParameter* sua_cfg_composite_param);
  void processSgpLinks(smsc::util::config::ConfigView& sua_layer_cfg,
                       utilx::runtime_cfg::CompositeParameter* sua_cfg_composite_param);

  smsc::logger::Logger* _logger;
};

}}}}

#endif
