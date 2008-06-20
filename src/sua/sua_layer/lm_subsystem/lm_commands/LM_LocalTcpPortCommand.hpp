#ifndef __SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMLOCALTCPPORTCOMMAND_HPP__
# define __SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMLOCALTCPPORTCOMMAND_HPP__

# include <netinet/in.h>
# include <sua/sua_layer/lm_subsystem/lm_commands/LM_Command.hpp>

namespace lm_commands {

class LM_LocalTcpPortCommand : public LM_Command {
public:
  explicit LM_LocalTcpPortCommand(const std::string& localPortValue);
  virtual std::string executeCommand();
private:
  in_port_t _localPort;
};

}

#endif
