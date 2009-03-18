template<class LOOKED_FOR_PARAMETER_TYPE>
bool
MessagesRouterSubsystem::checkParameterValueIsPresentInConfig(const std::string& parentParamName,
                                                              const std::string& interestedParamName,
                                                              const std::string& valueToCompare)
{
  utilx::runtime_cfg::RuntimeConfig& runtimeConfig = runtime_cfg::RuntimeConfig::getInstance();
  utilx::runtime_cfg::CompositeParameter& parent_parameter = runtimeConfig.find<utilx::runtime_cfg::CompositeParameter>(parentParamName);

  smsc_log_debug(_logger, "MessagesRouterSubsystem::checkParameterValueIsPresentInConfig::: parentParamName=%s,interestedParamName=%s,valueToCompare=%s",parentParamName.c_str(), interestedParamName.c_str(), valueToCompare.c_str());
  utilx::runtime_cfg::CompositeParameter::Iterator<LOOKED_FOR_PARAMETER_TYPE> iterator = parent_parameter.getIterator<LOOKED_FOR_PARAMETER_TYPE>(interestedParamName);
  while(iterator.hasElement()) {
    const LOOKED_FOR_PARAMETER_TYPE* interestedParameter = iterator.getCurrentElement();
    smsc_log_debug(_logger, "MessagesRouterSubsystem::checkParameterValueIsPresentInConfig::: next interestedParameter value=[%s]", interestedParameter->getValue().c_str());
    if ( valueToCompare == interestedParameter->getValue() ) return true;
    iterator.next();
  }

  return false;
}
