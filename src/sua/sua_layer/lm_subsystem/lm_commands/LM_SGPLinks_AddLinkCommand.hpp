#ifndef __SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMSGPLINKSADDLINKCOMMAND_HPP__
# define __SUA_SUALAYER_LMSUBSYSTEM_LMCOMMANDS_LMSGPLINKSADDLINKCOMMAND_HPP__

# include <vector>
# include <netinet/in.h>
# include <sua/sua_layer/lm_subsystem/lm_commands/LM_Command.hpp>

namespace lm_commands {

class LM_SGPLinks_AddLinkCommand : public LM_Command {
public:
  LM_SGPLinks_AddLinkCommand(const std::string& linkId,
                             const std::vector<std::string>& localAddressList, in_port_t localPort,
                             const std::vector<std::string>& remoteAddressList, in_port_t remotePort);

  virtual std::string executeCommand();
private:
  std::string _linkId;
  std::vector<std::string> _localAddressList, _remoteAddressList;
  in_port_t _localPort, _remotePort;
};

}

#endif
