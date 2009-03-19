#include "RuntimeConfig.hpp"
#include <stdexcept>
#include <memory>
#include <set>

namespace eyeline {
namespace utilx {
namespace runtime_cfg {

RuntimeConfig::RuntimeConfig()
  : _config(new CompositeParameter("root")), _wasInitialized(false)
{}

void
RuntimeConfig::initialize(CompositeParameter* fullConfiguration)
{
  if ( !_wasInitialized ) {
    _config->addParameter(fullConfiguration);
    _wasInitialized = true;
  }
}

CompositeParameter&
RuntimeConfig::findLastNodeParameter(const std::string& parameterName, std::string* leafParameterName)
{
  std::string::size_type start_token_idx = 0, end_token_idx = parameterName.find('.');
  CompositeParameter* nodeParameter = _config;
  while (end_token_idx != std::string::npos) {
    std::string token = parameterName.substr(start_token_idx, end_token_idx - start_token_idx);

    nodeParameter = nodeParameter->getParameter<CompositeParameter>(token);
    if ( !nodeParameter )
      throw std::runtime_error(std::string("RuntimeConfig::findLastNodeParameter::: node parameter ") + parameterName + std::string(" is not found"));
    start_token_idx = end_token_idx+1; end_token_idx = parameterName.find('.', start_token_idx);
  }
  *leafParameterName = parameterName.substr(start_token_idx, parameterName.size() - start_token_idx);

  return *nodeParameter;
}

void
RuntimeConfig::registerParameterObserver(const std::string& fullParameterName, ParameterObserver* handler)
{
  registeredParameterHandlers_t::iterator iter = _registredParameterHandlers.find(fullParameterName);
  if ( iter == _registredParameterHandlers.end() )
    _registredParameterHandlers.insert(std::make_pair(fullParameterName, handler));
  else
    iter->second = handler;
}

void
RuntimeConfig::notifyAddParameterEvent(const CompositeParameter& context, Parameter* addedParameter)
{
  std::string fullAddedParameterName = context.getFullName() + "." + addedParameter->getName();
  registeredParameterHandlers_t::iterator iter = _registredParameterHandlers.find(fullAddedParameterName);

  if ( iter != _registredParameterHandlers.end() ) {
    iter->second->addParameterEventHandler(context, addedParameter);
  }
}

CompositeParameter*
RuntimeConfig::notifyAddParameterEvent(const CompositeParameter& context, CompositeParameter* addedParameter)
{
  std::string fullAddedParameterName = context.getFullName() + "." + addedParameter->getName();
  registeredParameterHandlers_t::iterator iter = _registredParameterHandlers.find(fullAddedParameterName);

  if ( iter != _registredParameterHandlers.end() )
    return iter->second->addParameterEventHandler(context, addedParameter);
  else
    return NULL;
}

void
RuntimeConfig::notifyAddParameterEvent(CompositeParameter* context, Parameter* addedParameter)
{
  std::string fullAddedParameterName = context->getFullName() + "." + addedParameter->getName();
  registeredParameterHandlers_t::iterator iter = _registredParameterHandlers.find(fullAddedParameterName);

  if ( iter != _registredParameterHandlers.end() ) {
    iter->second->addParameterEventHandler(context, addedParameter);
  }
}

void
RuntimeConfig::notifyChangeParameterEvent(const CompositeParameter& context, const Parameter& modifiedParameter)
{
  std::string fullModifiedParameterName = context.getFullName() + "." + modifiedParameter.getName();
  registeredParameterHandlers_t::iterator iter = _registredParameterHandlers.find(fullModifiedParameterName);

  if ( iter != _registredParameterHandlers.end() ) {
    iter->second->changeParameterEventHandler(context, modifiedParameter);
  }
}

void
RuntimeConfig::notifyChangeParameterEvent(CompositeParameter* context, const Parameter& modifiedParameter)
{
  std::string fullModifiedParameterName = context->getFullName() + "." + modifiedParameter.getName();
  registeredParameterHandlers_t::iterator iter = _registredParameterHandlers.find(fullModifiedParameterName);

  if ( iter != _registredParameterHandlers.end() ) {
    iter->second->changeParameterEventHandler(context, modifiedParameter);
  }
}

void
RuntimeConfig::notifyRemoveParameterEvent(const Parameter& removedParameter)
{
  registeredParameterHandlers_t::iterator iter = _registredParameterHandlers.find(removedParameter.getFullName());

  if ( iter != _registredParameterHandlers.end() ) {
    iter->second->removeParameterEventHandler(removedParameter);
  }
}

void
RuntimeConfig::notifyRemoveParameterEvent(const CompositeParameter& context, const Parameter& removedParameter)
{
  std::string fullRemovedParameterName = context.getFullName() + "." + removedParameter.getName();
  registeredParameterHandlers_t::iterator iter = _registredParameterHandlers.find(fullRemovedParameterName);

  if ( iter != _registredParameterHandlers.end() ) {
    iter->second->removeParameterEventHandler(context, removedParameter);
  }
}

void
RuntimeConfig::addParameter(CompositeParameter* compositeParameter, const std::string& paramName, const std::string& paramValue)
{
  std::string::size_type pos, old_pos=0;
  while( (pos=paramValue.find_first_of(", ", old_pos)) != std::string::npos) {
    const std::string& nextValue = paramValue.substr(old_pos, pos - old_pos);

    compositeParameter->addParameter(new Parameter(paramName, nextValue));
    ++pos;
    while (paramValue[pos] == ' ') pos++;
    old_pos = pos;
  }
  const std::string& nextValue = paramValue.substr(old_pos);
  compositeParameter->addParameter(new Parameter(paramName, nextValue));
}

std::string
RuntimeConfig::printConfig() const
{
  return _config->printParamaterValue();
}

}}}
