#include "LM_Applications_AddApplicationCommand.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

std::string
LM_Applications_AddApplicationCommand::executeCommand()
{
  smsc_log_debug(_logger, "LM_Applications_AddApplicationCommand::executeCommand::: _appId=%s", _appId.c_str());
  return "OK";
}

}}}}}
