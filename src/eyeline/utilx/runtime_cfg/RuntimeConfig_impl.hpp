template <> inline
CompositeParameter&
RuntimeConfig::find<CompositeParameter>(const std::string& parameterName) {
  std::string leafParameterName;
  CompositeParameter& nodeParameter = findLastNodeParameter(parameterName, &leafParameterName);

  CompositeParameter* nodeParameterPtr = nodeParameter.getParameter<CompositeParameter>(leafParameterName);
  if (  !nodeParameterPtr )
    throw std::runtime_error(std::string("RuntimeConfig::find<CompositeParameter>::: node parameter ") + parameterName + std::string(" is not found"));

  return *nodeParameterPtr;

}

template <> inline
Parameter&
RuntimeConfig::find<Parameter>(const std::string& parameterName) {
  std::string leafParameterName;
  CompositeParameter& nodeParameter = findLastNodeParameter(parameterName, &leafParameterName);

  Parameter* childParameterPtr = nodeParameter.getParameter<Parameter>(leafParameterName);
  if (  !childParameterPtr )
    throw std::runtime_error(std::string("RuntimeConfig::find<Parameter>::: leaf parameter ") + parameterName + std::string(" is not found"));

  return *childParameterPtr;
}
