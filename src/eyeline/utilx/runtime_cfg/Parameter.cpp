#include "Parameter.hpp"
#include <stdio.h>
#include <stdexcept>

namespace eyeline {
namespace utilx {
namespace runtime_cfg {

Parameter::Parameter(const std::string& name, const std::string& value)
  : _name(name), _fullName(name), _value(value), _isSetValue(true), _isSetIntValue(false) {}

Parameter::Parameter(const std::string& name, int value)
  : _name(name), _fullName(name), _intValue(value), _isSetIntValue(true)
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
  if ( !_isSetValue )
    throw std::runtime_error("Parameter::getValue::: value isn't set");
  return _value;
}

int
Parameter::getIntValue() const
{
  if ( !_isSetIntValue )
    throw std::runtime_error("Parameter::getIntValue::: integer value isn't set");
  return _intValue;
}

void
Parameter::setValue(const std::string& value)
{
  _value = value; _isSetValue = true;
  _intValue = 0; _isSetIntValue = false;
}

void
Parameter::setIntValue(int value)
{
  _value = ""; _isSetValue = false;
  _intValue = value; _isSetIntValue = true;
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

}}}
