#include "LM_SGPLinks_RemoveLinkCommand.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

std::string
LM_SGPLinks_RemoveLinkCommand::executeCommand()
{
  smsc_log_debug(_logger, "LM_SGPLinks_RemoveLinkCommand::executeCommand::: linkId=[%s]", _linkId.c_str());
  return "OK";
}

}}}}}
