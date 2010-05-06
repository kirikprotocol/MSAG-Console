#ifndef __EYELINECOM_SS7NA_COMMON_APPLICATIONSUBSYSTEM_HPP__
# define __EYELINECOM_SS7NA_COMMON_APPLICATIONSUBSYSTEM_HPP__

# include <string>
# include "logger/Logger.h"
# include "eyeline/utilx/Subsystem.hpp"
# include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"
# include "eyeline/utilx/runtime_cfg/CompositeParameter.hpp"

namespace eyeline {
namespace ss7na {
namespace common {

class ApplicationSubsystem : public utilx::Subsystem {
public:
  virtual void start() {}
  virtual void stop() {}
  virtual void waitForCompletion() {}

  virtual const std::string& getName() const {
    return _name;
  }

  virtual void notifyLinkShutdownCompletion() {}

protected:
  ApplicationSubsystem(const std::string& subsystem_name,
                       const std::string& susbsystem_logger_name)
  : _logger(smsc::logger::Logger::getInstance(susbsystem_logger_name.c_str())),
    _name(subsystem_name)
  {}

  template<class PARAMETER_TYPE>
  bool checkParameterExist(utilx::runtime_cfg::CompositeParameter* composite_param,
                           const PARAMETER_TYPE& added_param);

  void checkConsistentRuntimeCommand(utilx::runtime_cfg::CompositeParameter* compostite_param_in_runtime_cfg,
                                     const std::string& requested_param,
                                     const std::string& conflicted_param);

  utilx::runtime_cfg::CompositeParameter*
  findContextParentParameter(utilx::runtime_cfg::RuntimeConfig& runtime_config,
                             const utilx::runtime_cfg::CompositeParameter& context);

  smsc::logger::Logger* _logger;
  std::string _name;
};

template<class PARAMETER_TYPE>
bool
ApplicationSubsystem::checkParameterExist(utilx::runtime_cfg::CompositeParameter* composite_param,
                                          const PARAMETER_TYPE& added_param)
{
  utilx::runtime_cfg::CompositeParameter::Iterator<PARAMETER_TYPE> existenceParamIterator = composite_param->getIterator<PARAMETER_TYPE>(added_param.getName());

  while ( existenceParamIterator.hasElement() ) {
    if ( existenceParamIterator.getCurrentElement()->getValue() == added_param.getValue() )
      return true;

    existenceParamIterator.next();
  }
  return false;
}

}}}

#endif
