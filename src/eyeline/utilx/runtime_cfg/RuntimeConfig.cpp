#include "RuntimeConfig.hpp"
#include <stdexcept>
#include <memory>
#include <set>
#include "eyeline/utilx/strtol.hpp"

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
      throw std::runtime_error("RuntimeConfig::findLastNodeParameter::: node parameter " + param_name + " is not found");
    start_token_idx = end_token_idx+1; end_token_idx = param_name.find('.', start_token_idx);
  }
  *leaf_param_name = param_name.substr(start_token_idx, param_name.size() - start_token_idx);

  return *nodeParameter;
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

void
RuntimeConfig::parseCommaSeparatedIntValue(const std::string& comma_sep_value,
                                           utilx::runtime_cfg::CompositeParameter* composite_param,
                                           const char* param_name)
{
  std::string::size_type pos, old_pos=0;
  while( (pos=comma_sep_value.find_first_of(", ", old_pos)) != std::string::npos) {
    const std::string& nextValue = comma_sep_value.substr(old_pos, pos - old_pos);
    int value = static_cast<int>(utilx::strtol(nextValue.c_str(), NULL, 10));
    if ( !value && errno )
      throw std::runtime_error("RuntimeConfig::parseCommaSeparatedIntValue::: invalid number format='" + nextValue + "'");
    composite_param->addParameter(new utilx::runtime_cfg::Parameter(param_name,
                                                                    value));
    ++pos;
    while (comma_sep_value[pos] == ' ') pos++;
    old_pos = pos;
  }
  const std::string& nextValue = comma_sep_value.substr(old_pos, pos - old_pos);
  int value = static_cast<int>(utilx::strtol(nextValue.c_str(), NULL, 10));
  if ( !value && errno )
    throw std::runtime_error("RuntimeConfig::parseCommaSeparatedIntValue::: invalid number format='" + nextValue + "'");
  composite_param->addParameter(new utilx::runtime_cfg::Parameter(param_name,
                                                                  value));
}

}}}
