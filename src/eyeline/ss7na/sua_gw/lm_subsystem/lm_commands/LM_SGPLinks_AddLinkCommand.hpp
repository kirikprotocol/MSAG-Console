#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMSGPLINKSADDLINKCOMMAND_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMSGPLINKSADDLINKCOMMAND_HPP__

# include <vector>
# include <netinet/in.h>
# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_SGPLinks_AddLinkCommand : public common::lm_subsystem::LM_Command {
public:
  LM_SGPLinks_AddLinkCommand(const std::string& link_id,
                             const std::vector<std::string>& local_addr_list, in_port_t local_port,
                             const std::vector<std::string>& remote_addr_list, in_port_t remote_port);

  virtual std::string executeCommand();

private:
  std::string _linkId;
  std::vector<std::string> _localAddressList, _remoteAddressList;
  in_port_t _localPort, _remotePort;
};

}}}}}

#endif
