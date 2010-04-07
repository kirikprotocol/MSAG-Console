#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMLOCALTCPPORTCOMMAND_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMLOCALTCPPORTCOMMAND_HPP__

# include <netinet/in.h>
# include "eyeline/ss7na/sua_gw/lm_subsystem/lm_commands/LM_Command.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_LocalTcpPortCommand : public LM_Command {
public:
  explicit LM_LocalTcpPortCommand(const std::string& localPortValue);
  virtual std::string executeCommand();
private:
  in_port_t _localPort;
};

}}}}}

#endif
