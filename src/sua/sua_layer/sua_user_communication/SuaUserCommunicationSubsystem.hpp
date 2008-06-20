#ifndef __SUA_SUALAYER_SUAUSERCOMMUNICATION_SUAUSERCOMMUNICATIONSUBSYSTEM_HPP__
# define __SUA_SUALAYER_SUAUSERCOMMUNICATION_SUAUSERCOMMUNICATIONSUBSYSTEM_HPP__ 1

# include <string>

# include <sua/utilx/Singleton.hpp>
# include <sua/sua_layer/ApplicationSubsystem.hpp>
# include <sua/sua_layer/runtime_cfg/RuntimeConfig.hpp>
# include <sua/sua_layer/runtime_cfg/ParameterObserver.hpp>

namespace sua_user_communication {

class SuaUserCommunicationSubsystem : public sua_layer::ApplicationSubsystem,
                                      public runtime_cfg::ParameterObserver,
                                      public utilx::Singleton<SuaUserCommunicationSubsystem*> {
public:
  SuaUserCommunicationSubsystem();
  virtual void stop();
  virtual void initialize(runtime_cfg::RuntimeConfig& rconfig);

  using ParameterObserver::addParameterEventHandler;
  using ParameterObserver::changeParameterEventHandler;
  //  using ParameterObserver::removeParameterEventHandler;

  virtual void addParameterEventHandler(const runtime_cfg::CompositeParameter& context,
                                        runtime_cfg::Parameter* addedParameter);
  virtual void changeParameterEventHandler(const runtime_cfg::CompositeParameter& context,
                                           const runtime_cfg::Parameter& modifiedParameter);
  //  virtual void removeParameterEventHandler(const runtime_cfg::Parameter& removedParameter);
private:
  char _acceptorName[128];
};

}

#endif
