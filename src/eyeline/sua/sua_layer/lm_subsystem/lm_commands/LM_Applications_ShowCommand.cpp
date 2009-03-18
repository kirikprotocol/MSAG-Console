#include "LM_Applications_ShowCommand.hpp"

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace lm_subsystem {
namespace lm_commands {

std::string
LM_Applications_ShowCommand::executeCommand()
{
  smsc_log_debug(_logger, "called LM_Applications_ShowCommand::executeCommand:::");
  return "OK";
}

}}}}}
