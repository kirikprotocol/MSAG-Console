#include <eyeline/utilx/runtime_cfg/Exception.hpp>
#include <eyeline/utilx/runtime_cfg/RuntimeConfig.hpp>
#include <eyeline/sua/sua_layer/runtime_cfg/RuntimeConfig.hpp>
#include "ApplicationSubsystem.hpp"

namespace eyeline {
namespace sua {
namespace sua_layer {

ApplicationSubsystem::ApplicationSubsystem(const std::string& subsystemName,
                                           const std::string& susbsystemLoggerName)
  : _logger(smsc::logger::Logger::getInstance(susbsystemLoggerName.c_str())), _name(subsystemName)  {}

const std::string&
ApplicationSubsystem::getName() const
{
  return _name;
}

void
ApplicationSubsystem::start()
{}

void
ApplicationSubsystem::stop()
{}

void
ApplicationSubsystem::waitForCompletion()
{}

template<class PARAMETER_TYPE>
bool
ApplicationSubsystem::checkParameterExist(utilx::runtime_cfg::CompositeParameter* compositeParam,
                                          PARAMETER_TYPE* addedParameter)
{
  utilx::runtime_cfg::CompositeParameter::Iterator<PARAMETER_TYPE> existenceParamIterator = compositeParam->getIterator<PARAMETER_TYPE>(addedParameter->getName());

  while ( existenceParamIterator.hasElement() ) {
    if ( existenceParamIterator.getCurrentElement()->getValue() == addedParameter->getValue() )
      return true;

    existenceParamIterator.next();
  }
  return false;
}

void
ApplicationSubsystem::checkConsistentRuntimeCommand(utilx::runtime_cfg::CompositeParameter* compostiteParamInRuntimeConfig,
                                                    const std::string& requestedParam,
                                                    const std::string& conflictedParam)
{
  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::Parameter> conflictedParamIter = compostiteParamInRuntimeConfig->getIterator<utilx::runtime_cfg::Parameter>(conflictedParam);
  if ( conflictedParamIter.hasElement() ) {
    std::string messageToUser("Inconsistent config modification request - there is '");
    messageToUser += conflictedParam + "' parameter";
    throw utilx::runtime_cfg::InconsistentConfigCommandException(messageToUser, "ApplicationSubsystem::addParameterEventHandler::: can't process parameter '%s' - '%s' parameter already exist", requestedParam.c_str(), conflictedParam.c_str());
  }
}

void
ApplicationSubsystem::generateExceptionAndForcePopUpCurrentInterpreter(const std::string& messageToUser,
                                                                       const char * fmt, ...)
{
  va_list arglist;
  va_start(arglist,fmt);
  std::string message;
  smsc::util::vformat(message, fmt, arglist);
  va_end(arglist);

  utilx::runtime_cfg::InconsistentConfigCommandException generatedException(messageToUser, message.c_str());
  generatedException.forcePopUpCurrentCommandInterpreter();
  throw generatedException;
}

utilx::runtime_cfg::CompositeParameter*
ApplicationSubsystem::findContexParentParameter(const utilx::runtime_cfg::CompositeParameter& context)
{
  utilx::runtime_cfg::RuntimeConfig& runtimeConfig = runtime_cfg::RuntimeConfig::getInstance();

  const std::string& contextName =  context.getFullName();
  std::string::size_type idx = contextName.rfind(".");
  const std::string& contextParentParameterName = contextName.substr(0, idx);
  const std::string& contextParameterName = contextName.substr(idx+1);
  utilx::runtime_cfg::CompositeParameter& contextParentParam = runtimeConfig.find<utilx::runtime_cfg::CompositeParameter>(contextParentParameterName);

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
