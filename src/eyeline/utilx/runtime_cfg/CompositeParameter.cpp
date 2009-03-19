#include "CompositeParameter.hpp"
#include "RuntimeConfig.hpp"

namespace eyeline {
namespace utilx {
namespace runtime_cfg {

CompositeParameter::CompositeParameter(const std::string& name)
  : Parameter(name) {}

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
}

CompositeParameter*
CompositeParameter::addParameter(CompositeParameter* parameter)
{
  _compositeParameters.insert(std::make_pair(parameter->getName(), parameter));
  parameter->setParameterPrefix(getFullName()+".");
  return parameter;
}

std::string
CompositeParameter::printParamaterValue() const
{
  std::string printableValue;
  if ( isSetValue() )
    printableValue = Parameter::printParamaterValue() + "\n";

  for(composite_parameters_t::const_iterator iter = _compositeParameters.begin(), end_iter = _compositeParameters.end();
      iter != end_iter; ++iter) {
    printableValue += iter->second->printParamaterValue();
  }
  for(parameters_t::const_iterator iter = _parameters.begin(), end_iter = _parameters.end();
      iter != end_iter; ++iter) {
    printableValue += iter->second->printParamaterValue() + "\n";
  }
  return printableValue;
}

}}}
