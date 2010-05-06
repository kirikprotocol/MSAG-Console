#ifndef __EYELINE_UTILX_RUNTIMECFG_RUNTIMECONFIG_HPP__
# define __EYELINE_UTILX_RUNTIMECFG_RUNTIMECONFIG_HPP__

# include <string>
# include <map>
# include <util/config/ConfigView.h>
# include "eyeline/utilx/runtime_cfg/CompositeParameter.hpp"
# include "eyeline/utilx/Singleton.hpp"

namespace eyeline {
namespace utilx {
namespace runtime_cfg {

class RuntimeConfig {
public:
  RuntimeConfig();
  virtual ~RuntimeConfig() {}

  void initialize(CompositeParameter* full_configuration);
  virtual void initialize(smsc::util::config::ConfigView& xmlConfig,
                          const char* cfg_file) = 0;

  template <class PARAMETER_TYPE> PARAMETER_TYPE& find(const std::string& param_name);

  std::string printConfig() const;

protected:
  CompositeParameter& findLastNodeParameter(const std::string& param_name,
                                            std::string* leaf_param_name);
  void addParameter(CompositeParameter* composite_param,
                    const std::string& param_name,
                    const std::string& param_value);
  void parseCommaSeparatedValue(const std::string& comma_sep_value,
                                utilx::runtime_cfg::CompositeParameter* composite_param,
                                const char* param_name);
  void parseCommaSeparatedIntValue(const std::string& comma_sep_value,
                                   utilx::runtime_cfg::CompositeParameter* composite_param,
                                   const char* param_name);

private:
  CompositeParameter* _config;

  bool _wasInitialized;
};

# include <eyeline/utilx/runtime_cfg/RuntimeConfig_impl.hpp>

}}}

#endif
