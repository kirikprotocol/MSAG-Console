#ifndef RUNTIMECONFIG_HPP_HEADER_INCLUDED_B8C7BC1F
# define RUNTIMECONFIG_HPP_HEADER_INCLUDED_B8C7BC1F

# include <string>
# include <map>
# include <util/config/ConfigView.h>
# include <logger/Logger.h>
# include <sua/sua_layer/runtime_cfg/CompositeParameter.hpp>
# include <sua/sua_layer/runtime_cfg/ParameterObserver.hpp>
# include <sua/utilx/Singleton.hpp>

namespace runtime_cfg {

class RuntimeConfig : public utilx::Singleton<RuntimeConfig> {
public:
  RuntimeConfig();

  void initialize(CompositeParameter* fullConfiguration);
  void initialize(smsc::util::config::ConfigView* xmlConfig);
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
private:
  CompositeParameter& findLastNodeParameter(const std::string& parameterName, std::string* leafParameterName);
  void processRoutingKeysSection(smsc::util::config::ConfigView* suaLayerCfg,
                                 const char* rkSectionName, // e.g. "incoming-routing-keys"
                                 CompositeParameter* suaConfigCompositeParameter);
  void addParameter(CompositeParameter* compositeParameter, const std::string& paramName, const std::string& paramValue);

  CompositeParameter* _config;
  typedef std::map<std::string, ParameterObserver*> registeredParameterHandlers_t;

  registeredParameterHandlers_t _registredParameterHandlers;

  bool _wasInitialized;
  smsc::logger::Logger* _logger;
};

# include <sua/sua_layer/runtime_cfg/RuntimeConfig_impl.hpp>

}

#endif /* RUNTIMECONFIG_HPP_HEADER_INCLUDED_B8C7BC1F */
