#ifndef __EYELINE_SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMLOCALTCPPORTCOMMAND_HPP__
# define __EYELINE_SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMLOCALTCPPORTCOMMAND_HPP__

# include <netinet/in.h>
# include <eyeline/sua/sua_layer/lm_subsystem/lm_commands/LM_Command.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
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
