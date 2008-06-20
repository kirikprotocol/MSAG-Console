template<> inline
const CompositeParameter*
CompositeParameter::getParameter<CompositeParameter>(const std::string& parameterName) const
{
  composite_parameters_t::const_iterator iter = _compositeParameters.find(parameterName);
  if ( iter == _compositeParameters.end() ) return NULL;
  return iter->second;
}

template<> inline
const Parameter*
CompositeParameter::getParameter<Parameter>(const std::string& parameterName) const
{
  parameters_t::const_iterator iter = _parameters.find(parameterName);
  if ( iter == _parameters.end() ) return NULL;
  return iter->second;
}

template<> inline
CompositeParameter*
CompositeParameter::getParameter<CompositeParameter>(const std::string& parameterName)
{
  composite_parameters_t::const_iterator iter = _compositeParameters.find(parameterName);
  if ( iter == _compositeParameters.end() ) return NULL;
  return iter->second;
}

template<> inline
Parameter*
CompositeParameter::getParameter<Parameter>(const std::string& parameterName)
{
  parameters_t::const_iterator iter = _parameters.find(parameterName);
  if ( iter == _parameters.end() ) return NULL;
  return iter->second;
}

template<> inline
CompositeParameter::Iterator<Parameter>
CompositeParameter::getIterator<Parameter>(const std::string& parameterName)
{
  std::pair<parameters_t::iterator,  parameters_t::iterator> interval = _parameters.equal_range(parameterName);
  return Iterator<Parameter>(interval.first, interval.second);
}

template<> inline
CompositeParameter::Iterator<CompositeParameter>
CompositeParameter::getIterator<CompositeParameter>(const std::string& parameterName)
{
  std::pair<composite_parameters_t::iterator,  composite_parameters_t::iterator> interval = _compositeParameters.equal_range(parameterName);
  return Iterator<CompositeParameter>(interval.first, interval.second);
}
