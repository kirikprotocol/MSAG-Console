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
RuntimeConfig::initialize(CompositeParameter* full_configuration)
{
  if ( !_wasInitialized ) {
    _config->addParameter(full_configuration);
    _wasInitialized = true;
  }
}

CompositeParameter&
RuntimeConfig::findLastNodeParameter(const std::string& param_name,
                                     std::string* leaf_param_name)
{
  std::string::size_type start_token_idx = 0, end_token_idx = param_name.find('.');
  CompositeParameter* nodeParameter = _config;
  while (end_token_idx != std::string::npos) {
    std::string token = param_name.substr(start_token_idx, end_token_idx - start_token_idx);

    nodeParameter = nodeParameter->getParameter<CompositeParameter>(token);
    if ( !nodeParameter )
      throw std::runtime_error(std::string("RuntimeConfig::findLastNodeParameter::: node parameter ") + param_name + std::string(" is not found"));
    start_token_idx = end_token_idx+1; end_token_idx = param_name.find('.', start_token_idx);
  }
  *leaf_param_name = param_name.substr(start_token_idx, param_name.size() - start_token_idx);

  return *nodeParameter;
}

void
RuntimeConfig::registerParameterObserver(const std::string& full_param_name,
                                         ParameterObserver* handler)
{
  registeredParameterHandlers_t::iterator iter = _registredParameterHandlers.find(full_param_name);
  if ( iter == _registredParameterHandlers.end() )
    _registredParameterHandlers.insert(std::make_pair(full_param_name, handler));
  else
    iter->second = handler;
}

void
RuntimeConfig::notifyAddParameterEvent(const CompositeParameter& context,
                                       Parameter* added_param)
{
  std::string fullAddedParameterName = context.getFullName() + "." + added_param->getName();
  registeredParameterHandlers_t::iterator iter = _registredParameterHandlers.find(fullAddedParameterName);

  if ( iter != _registredParameterHandlers.end() ) {
    iter->second->addParameterEventHandler(context, added_param);
  }
}

CompositeParameter*
RuntimeConfig::notifyAddParameterEvent(const CompositeParameter& context,
                                       CompositeParameter* added_param)
{
  std::string fullAddedParameterName = context.getFullName() + "." + added_param->getName();
  registeredParameterHandlers_t::iterator iter = _registredParameterHandlers.find(fullAddedParameterName);

  if ( iter != _registredParameterHandlers.end() )
    return iter->second->addParameterEventHandler(context, added_param);
  else
    return NULL;
}

void
RuntimeConfig::notifyAddParameterEvent(CompositeParameter* context,
                                       Parameter* added_param)
{
  std::string fullAddedParameterName = context->getFullName() + "." + added_param->getName();
  registeredParameterHandlers_t::iterator iter = _registredParameterHandlers.find(fullAddedParameterName);

  if ( iter != _registredParameterHandlers.end() ) {
    iter->second->addParameterEventHandler(context, added_param);
  }
}

void
RuntimeConfig::notifyChangeParameterEvent(const CompositeParameter& context,
                                          const Parameter& modified_param)
{
  std::string fullModifiedParameterName = context.getFullName() + "." + modified_param.getName();
  registeredParameterHandlers_t::iterator iter = _registredParameterHandlers.find(fullModifiedParameterName);

  if ( iter != _registredParameterHandlers.end() ) {
    iter->second->changeParameterEventHandler(context, modified_param);
  }
}

void
RuntimeConfig::notifyChangeParameterEvent(CompositeParameter* context, const Parameter& modified_param)
{
  std::string fullModifiedParameterName = context->getFullName() + "." + modified_param.getName();
  registeredParameterHandlers_t::iterator iter = _registredParameterHandlers.find(fullModifiedParameterName);

  if ( iter != _registredParameterHandlers.end() ) {
    iter->second->changeParameterEventHandler(context, modified_param);
  }
}

void
RuntimeConfig::notifyRemoveParameterEvent(const Parameter& removed_param)
{
  registeredParameterHandlers_t::iterator iter = _registredParameterHandlers.find(removed_param.getFullName());

  if ( iter != _registredParameterHandlers.end() ) {
    iter->second->removeParameterEventHandler(removed_param);
  }
}

void
RuntimeConfig::notifyRemoveParameterEvent(const CompositeParameter& context,
                                          const Parameter& removed_param)
{
  std::string fullRemovedParameterName = context.getFullName() + "." + removed_param.getName();
  registeredParameterHandlers_t::iterator iter = _registredParameterHandlers.find(fullRemovedParameterName);

  if ( iter != _registredParameterHandlers.end() ) {
    iter->second->removeParameterEventHandler(context, removed_param);
  }
}

void
RuntimeConfig::addParameter(CompositeParameter* composite_param,
                            const std::string& param_name, const std::string& param_value)
{
  std::string::size_type pos, old_pos=0;
  while( (pos=param_value.find_first_of(", ", old_pos)) != std::string::npos) {
    const std::string& nextValue = param_value.substr(old_pos, pos - old_pos);

    composite_param->addParameter(new Parameter(param_name, nextValue));
    ++pos;
    while (param_value[pos] == ' ') pos++;
    old_pos = pos;
  }
  const std::string& nextValue = param_value.substr(old_pos);
  composite_param->addParameter(new Parameter(param_name, nextValue));
}

std::string
RuntimeConfig::printConfig() const
{
  return _config->printParamaterValue();
}

void
RuntimeConfig::parseCommaSeparatedValue(const std::string& comma_sep_value,
                                        utilx::runtime_cfg::CompositeParameter* composite_param,
                                        const char* param_name)
{
  std::string::size_type pos, old_pos=0;
  while( (pos=comma_sep_value.find_first_of(", ", old_pos)) != std::string::npos) {
    composite_param->addParameter(new utilx::runtime_cfg::Parameter(param_name,
                                                                    comma_sep_value.substr(old_pos, pos - old_pos)));
    ++pos;
    while (comma_sep_value[pos] == ' ') pos++;
    old_pos = pos;
  }
  composite_param->addParameter(new utilx::runtime_cfg::Parameter(param_name,
                                                                  comma_sep_value.substr(old_pos)));
}

}}}
