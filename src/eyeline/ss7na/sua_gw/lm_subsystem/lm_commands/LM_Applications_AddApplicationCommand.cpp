#include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"
#include "eyeline/ss7na/sua_gw/runtime_cfg/RuntimeConfig.hpp"

#include "LM_Applications_AddApplicationCommand.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

LM_Applications_AddApplicationCommand::LM_Applications_AddApplicationCommand(const std::string& appIdValue)
  : _appId(appIdValue)
{}

std::string
LM_Applications_AddApplicationCommand::executeCommand()
{
  smsc_log_debug(_logger, "LM_Applications_AddApplicationCommand::executeCommand::: _appId=%s", _appId.c_str());
  runtime_cfg::RuntimeConfig::getInstance().notifyAddParameterEvent(utilx::runtime_cfg::CompositeParameter("config.sua_applications"),
                                                                    new utilx::runtime_cfg::Parameter("application", _appId));
  return "OK";
}

}}}}}
