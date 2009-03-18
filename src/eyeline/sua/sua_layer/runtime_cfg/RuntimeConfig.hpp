#ifndef __EYELINE_SUA_SUALAYER_RUNTIMECFG_RUNTIMECONFIG_HPP__
# define __EYELINE_SUA_SUALAYER_RUNTIMECFG_RUNTIMECONFIG_HPP__

# include <string>
# include <map>
# include <util/config/ConfigView.h>
# include <logger/Logger.h>
# include <eyeline/utilx/Singleton.hpp>
# include <eyeline/utilx/runtime_cfg/RuntimeConfig.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace runtime_cfg {

class RuntimeConfig : public utilx::Singleton<RuntimeConfig>,
                      public utilx::runtime_cfg::RuntimeConfig {
public:
  RuntimeConfig();

  using utilx::runtime_cfg::RuntimeConfig::initialize;

  virtual void initialize(smsc::util::config::ConfigView* xmlConfig);
private:
  void processRoutingKeysSection(smsc::util::config::ConfigView* suaLayerCfg,
                                 const char* rkSectionName, // e.g. "incoming-routing-keys"
                                 utilx::runtime_cfg::CompositeParameter* suaConfigCompositeParameter);

  smsc::logger::Logger* _logger;
};

}}}}

#endif
