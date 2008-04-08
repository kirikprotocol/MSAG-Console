#ifndef __SUA_SUALAYER_SUAUSERCOMMUNICATION_SUAUSERCOMMUNICATIONSUBSYSTEM_HPP__
# define __SUA_SUALAYER_SUAUSERCOMMUNICATION_SUAUSERCOMMUNICATIONSUBSYSTEM_HPP__ 1

# include <string>
# include <sua/utilx/Subsystem.hpp>
# include <sua/utilx/Singleton.hpp>
# include <util/config/ConfigView.h>
# include <logger/Logger.h>
# include <netinet/in.h>

namespace sua_user_communication {

class SuaUserCommunicationSubsystem : public utilx::Subsystem,
                                      public utilx::Singleton<SuaUserCommunicationSubsystem*> {
public:
  SuaUserCommunicationSubsystem();
  virtual void start();
  virtual void stop();
  virtual void initialize(runtime_cfg::RuntimeConfig& rconfig);
  virtual void waitForCompletion();
  virtual const std::string& getName() const;
private:
  std::string _name;
  char _acceptorName[128];
  smsc::logger::Logger* _logger;
};

}

#endif
