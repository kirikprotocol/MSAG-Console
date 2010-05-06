#include "LM_SGPLinks_AddLinkCommand.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

LM_SGPLinks_AddLinkCommand::LM_SGPLinks_AddLinkCommand(const std::string& link_id,
                                                       const std::vector<std::string>& local_addr_list,
                                                       in_port_t local_port,
                                                       const std::vector<std::string>& remote_addr_list,
                                                       in_port_t remote_port)
  : _linkId(link_id),
    _localAddressList(local_addr_list), _localPort(local_port),
    _remoteAddressList(remote_addr_list), _remotePort(remote_port)
{}

std::string
LM_SGPLinks_AddLinkCommand::executeCommand()
{
  return "OK";
}

}}}}}
