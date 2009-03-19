#ifndef __EYELINE_UTILX_RUNTIMECFG_RUNTIMECONFIG_HPP__
# define __EYELINE_UTILX_RUNTIMECFG_RUNTIMECONFIG_HPP__

# include <string>
# include <map>
# include <util/config/ConfigView.h>
# include <eyeline/utilx/runtime_cfg/CompositeParameter.hpp>
# include <eyeline/utilx/runtime_cfg/ParameterObserver.hpp>
# include <eyeline/utilx/Singleton.hpp>

namespace eyeline {
namespace utilx {
namespace runtime_cfg {

class RuntimeConfig {
public:
  RuntimeConfig();
  virtual ~RuntimeConfig() {}

  void initialize(CompositeParameter* fullConfiguration);
  virtual void initialize(smsc::util::config::ConfigView* xmlConfig) = 0;

  template <class PARAMETER_TYPE> PARAMETER_TYPE& find(const std::string& parameterName);
  void registerParameterObserver(const std::string& parameterName, ParameterObserver* handler);

  void notifyAddParameterEvent(const CompositeParameter& context, Parameter* addedParameter);
  CompositeParameter* notifyAddParameterEvent(const CompositeParameter& context, CompositeParameter* addedParameter);
  void notifyAddParameterEvent(CompositeParameter* context, Parameter* addedParameter);

  void notifyChangeParameterEvent(const CompositeParameter& context, const Parameter& modifiedParameter);
  void notifyChangeParameterEvent(CompositeParameter* context, const Parameter& modifiedParameter);

  void notifyRemoveParameterEvent(const Parameter& modifiedParameter);
  void notifyRemoveParameterEvent(const CompositeParameter& context, const Parameter& modifiedParameter);

  std::string printConfig() const;

protected:
  CompositeParameter& findLastNodeParameter(const std::string& parameterName, std::string* leafParameterName);
  void addParameter(CompositeParameter* compositeParameter, const std::string& paramName, const std::string& paramValue);

private:
  CompositeParameter* _config;
  typedef std::map<std::string, ParameterObserver*> registeredParameterHandlers_t;

  registeredParameterHandlers_t _registredParameterHandlers;

  bool _wasInitialized;
};

# include <eyeline/utilx/runtime_cfg/RuntimeConfig_impl.hpp>

}}}

#endif
