#ifndef __EYELINE_SUA_SUALAYER_SUAUSERCOMMUNICATION_SUAUSERCOMMUNICATIONSUBSYSTEM_HPP__
# define __EYELINE_SUA_SUALAYER_SUAUSERCOMMUNICATION_SUAUSERCOMMUNICATIONSUBSYSTEM_HPP__

# include <string>

# include <eyeline/utilx/Singleton.hpp>
# include <eyeline/utilx/ThreadSpecificData.hpp>
# include <eyeline/utilx/runtime_cfg/ParameterObserver.hpp>
# include <eyeline/utilx/runtime_cfg/RuntimeConfig.hpp>

# include <eyeline/sua/sua_layer/ApplicationSubsystem.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace sua_user_communication {

class SuaUserCommunicationSubsystem : public sua_layer::ApplicationSubsystem,
                                      public utilx::runtime_cfg::ParameterObserver,
                                      public utilx::Singleton<SuaUserCommunicationSubsystem*> {
public:
  SuaUserCommunicationSubsystem();
  virtual void stop();
  virtual void initialize(utilx::runtime_cfg::RuntimeConfig& rconfig);

  using ParameterObserver::addParameterEventHandler;
  using ParameterObserver::changeParameterEventHandler;
  //  using ParameterObserver::removeParameterEventHandler;

  virtual void addParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                                        utilx::runtime_cfg::Parameter* addedParameter);
  virtual void changeParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                                           const utilx::runtime_cfg::Parameter& modifiedParameter);

private:
  void applyParametersChange();
  void initializeConnectAcceptor(const std::string& localHost,
                                 in_port_t localPort);

  static utilx::ThreadSpecificData<bool> _threadScopeModificationFlag;

  char _acceptorName[128];
};

}}}}

#endif
