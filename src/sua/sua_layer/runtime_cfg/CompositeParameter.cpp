#include "CompositeParameter.hpp"
#include "RuntimeConfig.hpp"

namespace runtime_cfg {

CompositeParameter::CompositeParameter(const std::string& name)
  : Parameter(name, "") {}

CompositeParameter::CompositeParameter(const std::string& name, const std::string& value)
  : Parameter(name, value) {}

CompositeParameter::~CompositeParameter()
{
  for(parameters_t::iterator iter = _parameters.begin(), end_iter = _parameters.end();
      iter != end_iter; ++iter) {
    delete iter->second;
  }
  for(composite_parameters_t::iterator iter = _compositeParameters.begin(), end_iter = _compositeParameters.end();
      iter != end_iter; ++iter) {
    delete iter->second;
  }
}

bool
CompositeParameter::isComposite() const { return true; }

void
CompositeParameter::addParameter(Parameter* parameter)
{
  _parameters.insert(std::make_pair(parameter->getName(), parameter));
  parameter->setParameterPrefix(getFullName()+".");
  RuntimeConfig::getInstance().dispatchHandle(*parameter);
}

CompositeParameter*
CompositeParameter::addParameter(CompositeParameter* parameter)
{
  _compositeParameters.insert(std::make_pair(parameter->getName(), parameter));
  parameter->setParameterPrefix(getFullName()+".");
  RuntimeConfig::getInstance().dispatchHandle(*parameter);
  return parameter;
}

}
