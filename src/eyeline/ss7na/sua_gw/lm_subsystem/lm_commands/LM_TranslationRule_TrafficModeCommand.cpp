#include "LM_TranslationRule_TrafficModeCommand.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

std::string
LM_TranslationRule_TrafficModeCommand::executeCommand()
{
  smsc_log_debug(_logger, "called LM_TranslationRule_TrafficModeCommand::executeCommand::: trafficMode=[%s]", _trafficMode.c_str());
  return "OK";
}

}}}}}
