#include "Parameter.hpp"
#include <stdio.h>
#include <stdexcept>
#include <iostream> // for debug

namespace runtime_cfg {

Parameter::Parameter(const std::string& name, const std::string& value)
  : _name(name), _fullName(name), _value(value), _isSetValue(true) {}

Parameter::Parameter(const std::string& name, unsigned int value)
  : _name(name), _fullName(name)
{
  char presentationBuf[32];
  snprintf(presentationBuf, sizeof(presentationBuf), "%d", value);
  _value.assign(presentationBuf);
  _isSetValue = true;
}

Parameter::Parameter(const std::string& name)
  : _name(name), _fullName(name), _isSetValue(false)
{}

const std::string&
Parameter::getName() const
{
  return _name;
}

const std::string&
Parameter::getFullName() const
{
  return _fullName;
}

bool
Parameter::isSetValue() const
{
  return _isSetValue;
}

const std::string&
Parameter::getValue() const
{
  return _value;
}

void
Parameter::setValue(const std::string& value)
{
  _value = value;
}

void
Parameter::setParameterPrefix(const std::string& parameterPrefix)
{
  _fullName = parameterPrefix + _name;
}

std::string
Parameter::printParamaterValue() const
{
  return
    _fullName + "='" +
    _value + "'";
}

}
