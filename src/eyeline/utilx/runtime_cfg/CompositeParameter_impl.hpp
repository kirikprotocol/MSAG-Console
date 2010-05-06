template<> inline
const CompositeParameter*
CompositeParameter::getParameter<CompositeParameter>(const std::string& param_name) const
{
  composite_parameters_t::const_iterator iter = _compositeParameters.find(param_name);
  if ( iter == _compositeParameters.end() ) return NULL;
  return iter->second;
}

template<> inline
const Parameter*
CompositeParameter::getParameter<Parameter>(const std::string& param_name) const
{
  parameters_t::const_iterator iter = _parameters.find(param_name);
  if ( iter == _parameters.end() ) return NULL;
  return iter->second;
}

template<> inline
CompositeParameter*
CompositeParameter::getParameter<CompositeParameter>(const std::string& param_name)
{
  composite_parameters_t::const_iterator iter = _compositeParameters.find(param_name);
  if ( iter == _compositeParameters.end() ) return NULL;
  return iter->second;
}

template<> inline
Parameter*
CompositeParameter::getParameter<Parameter>(const std::string& param_name)
{
  parameters_t::const_iterator iter = _parameters.find(param_name);
  if ( iter == _parameters.end() ) return NULL;
  return iter->second;
}

template<> inline
CompositeParameter::Iterator<Parameter>
CompositeParameter::getIterator<Parameter>(const std::string& param_name)
{
  std::pair<parameters_t::iterator,  parameters_t::iterator> interval = _parameters.equal_range(param_name);
  return Iterator<Parameter>(interval.first, interval.second, _parameters);
}

template<> inline
CompositeParameter::Iterator<CompositeParameter>
CompositeParameter::getIterator<CompositeParameter>(const std::string& param_name)
{
  std::pair<composite_parameters_t::iterator,  composite_parameters_t::iterator> interval = _compositeParameters.equal_range(param_name);
  return Iterator<CompositeParameter>(interval.first, interval.second, _compositeParameters);
}
