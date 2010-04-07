#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMLOCALIPCOMMAND_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMLOCALIPCOMMAND_HPP__

# include "eyeline/ss7na/sua_gw/lm_subsystem/lm_commands/LM_Command.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_LocalIpCommand : public LM_Command {
public:
  explicit LM_LocalIpCommand(const std::string& localHostValue);
  virtual std::string executeCommand();
private:
  std::string _localHost;
};

}}}}}

#endif
