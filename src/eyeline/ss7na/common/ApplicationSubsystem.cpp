#include "ApplicationSubsystem.hpp"
#include "eyeline/utilx/runtime_cfg/Exception.hpp"

namespace eyeline {
namespace ss7na {
namespace common {

void
ApplicationSubsystem::checkConsistentRuntimeCommand(utilx::runtime_cfg::CompositeParameter* compostite_param_in_runtime_cfg,
                                                    const std::string& requested_param,
                                                    const std::string& conflicted_param)
{
  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::Parameter> conflictedParamIter = compostite_param_in_runtime_cfg->getIterator<utilx::runtime_cfg::Parameter>(conflicted_param);
  if ( conflictedParamIter.hasElement() ) {
    std::string messageToUser("Inconsistent config modification request - there is '");
    messageToUser += conflicted_param + "' parameter";
    throw utilx::runtime_cfg::InconsistentConfigCommandException(messageToUser, "ApplicationSubsystem::addParameterEventHandler::: can't process parameter '%s' - '%s' parameter already exist", requested_param.c_str(), conflicted_param.c_str());
  }
}

utilx::runtime_cfg::CompositeParameter*
ApplicationSubsystem::findContextParentParameter(utilx::runtime_cfg::RuntimeConfig& runtime_config,
                                                 const utilx::runtime_cfg::CompositeParameter& context)
{
  const std::string& contextName =  context.getFullName();
  std::string::size_type idx = contextName.rfind(".");
  const std::string& contextParentParameterName = contextName.substr(0, idx);
  const std::string& contextParameterName = contextName.substr(idx+1);
  utilx::runtime_cfg::CompositeParameter& contextParentParam = runtime_config.find<utilx::runtime_cfg::CompositeParameter>(contextParentParameterName);

  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter> contextParamIterator = contextParentParam.getIterator<utilx::runtime_cfg::CompositeParameter>(contextParameterName);

  while (contextParamIterator.hasElement()) {
    utilx::runtime_cfg::CompositeParameter* contextParam = contextParamIterator.getCurrentElement();

    if ( contextParam && contextParam->getValue() == context.getValue() )
      return contextParam;

    contextParamIterator.next();
  }

  return NULL;
}

}}}
