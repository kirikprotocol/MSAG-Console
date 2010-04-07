#include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"
#include "eyeline/ss7na/sua_gw/runtime_cfg/RuntimeConfig.hpp"
#include "LM_TranslationRule_TrafficModeCommand.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

LM_TranslationRule_TrafficModeCommand::LM_TranslationRule_TrafficModeCommand(const std::string& trafficMode)
  : _trafficMode(trafficMode)
{}

std::string
LM_TranslationRule_TrafficModeCommand::executeCommand()
{
  smsc_log_debug(_logger, "called LM_TranslationRule_TrafficModeCommand::executeCommand::: trafficMode=[%s]", _trafficMode.c_str());

  runtime_cfg::RuntimeConfig::getInstance().notifyChangeParameterEvent(utilx::runtime_cfg::CompositeParameter("config.routing-keys.routingEntry", _ruleName), utilx::runtime_cfg::Parameter("traffic-mode", _trafficMode));

  return "OK";
}

void
LM_TranslationRule_TrafficModeCommand::setTranslationRuleName(const std::string& ruleName)
{
  _ruleName = ruleName;
}

}}}}}
