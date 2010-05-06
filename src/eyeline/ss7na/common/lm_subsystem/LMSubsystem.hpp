#ifndef __EYELINE_SS7NA_COMMON_LMSUBSYSTEM_LMSUBSYSTEM_HPP__
# define __EYELINE_SS7NA_COMMON_LMSUBSYSTEM_LMSUBSYSTEM_HPP__

# include <netinet/in.h>
# include <string>

# include "eyeline/ss7na/common/ApplicationSubsystem.hpp"
# include "eyeline/ss7na/common/lm_subsystem/InputCommandProcessor.hpp"
# include "eyeline/ss7na/common/lm_subsystem/UserInteractionProcessor.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace lm_subsystem {

class LMSubsystem : public common::ApplicationSubsystem {
public:
  LMSubsystem();
  using utilx::Subsystem::initialize;

  virtual void start();
  virtual void stop();
  virtual void waitForCompletion();

  void initialize(const std::string& host, in_port_t port,
                  InputCommandProcessor& input_cmd_processor);

private:
  UserInteractionProcessor* _userInteractionProcessor;
  std::string _host;
  in_port_t _port;
};

}}}}

#endif
