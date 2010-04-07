template <> inline
CompositeParameter&
RuntimeConfig::find<CompositeParameter>(const std::string& param_name) {
  std::string leafParameterName;
  CompositeParameter& nodeParameter = findLastNodeParameter(param_name, &leafParameterName);

  CompositeParameter* nodeParameterPtr = nodeParameter.getParameter<CompositeParameter>(leafParameterName);
  if (  !nodeParameterPtr )
    throw std::runtime_error(std::string("RuntimeConfig::find<CompositeParameter>::: node parameter ") + param_name + std::string(" is not found"));

  return *nodeParameterPtr;

}

template <> inline
Parameter&
RuntimeConfig::find<Parameter>(const std::string& param_name) {
  std::string leafParameterName;
  CompositeParameter& nodeParameter = findLastNodeParameter(param_name, &leafParameterName);

  Parameter* childParameterPtr = nodeParameter.getParameter<Parameter>(leafParameterName);
  if (  !childParameterPtr )
    throw std::runtime_error(std::string("RuntimeConfig::find<Parameter>::: leaf parameter ") + param_name + std::string(" is not found"));

  return *childParameterPtr;
}
