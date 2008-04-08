#include "Parameter.hpp"
#include <stdio.h>
#include <stdexcept>
#include <iostream> // for debug

namespace runtime_cfg {

Parameter::Parameter(const std::string& name, const std::string& value)
  : _name(name), _fullName(name), _value(value) {}

Parameter::Parameter(const std::string& name, unsigned int value)
  : _name(name), _fullName(name)
{
  char presentationBuf[32];
  snprintf(presentationBuf, sizeof(presentationBuf), "%d", value);
  _value.assign(presentationBuf);
}

//##ModelId=4737DFB50227
const
std::string& Parameter::getName() const
{
  return _name;
}

const
std::string& Parameter::getFullName() const
{
  return _fullName;
}

//##ModelId=4737DFA60289
const
std::string& Parameter::getValue() const
{
  return _value;
}

void
Parameter::setParameterPrefix(const std::string& parameterPrefix)
{
  _fullName = parameterPrefix + _name;
}

}
