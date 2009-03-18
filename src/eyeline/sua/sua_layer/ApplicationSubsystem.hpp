#ifndef __EYELINECOM_SUA_SUALAYER_APPLICATIONSUBSYSTEM_HPP__
# define __EYELINECOM_SUA_SUALAYER_APPLICATIONSUBSYSTEM_HPP__

# include <string>
# include <logger/Logger.h>
# include <eyeline/utilx/Subsystem.hpp>
# include <eyeline/utilx/runtime_cfg/CompositeParameter.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {

class ApplicationSubsystem : public utilx::Subsystem {
public:
  virtual void start();
  virtual void stop();
  virtual void waitForCompletion();

  virtual const std::string& getName() const;
protected:
  ApplicationSubsystem(const std::string& subsystemName, const std::string& susbsystemLoggerName);

  template<class PARAMETER_TYPE>
  bool checkParameterExist(utilx::runtime_cfg::CompositeParameter* compositeParam,
                           PARAMETER_TYPE* addedParameter);

  void checkConsistentRuntimeCommand(utilx::runtime_cfg::CompositeParameter* compostiteParamInRuntimeConfig,
                                     const std::string& requestedParam,
                                     const std::string& conflictedParam);

  utilx::runtime_cfg::CompositeParameter* findContexParentParameter(const utilx::runtime_cfg::CompositeParameter& context);

  void generateExceptionAndForcePopUpCurrentInterpreter(const std::string& messageToUser,
                                                        const char * fmt, ...);

  smsc::logger::Logger* _logger;

  std::string _name;
};

}}}

#endif
