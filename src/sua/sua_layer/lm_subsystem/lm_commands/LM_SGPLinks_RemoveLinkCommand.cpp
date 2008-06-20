#include <sua/sua_layer/runtime_cfg/RuntimeConfig.hpp>
#include "LM_SGPLinks_RemoveLinkCommand.hpp"

namespace lm_commands {

LM_SGPLinks_RemoveLinkCommand::LM_SGPLinks_RemoveLinkCommand(const std::string& linkId)
  : _linkId(linkId)
{}

std::string
LM_SGPLinks_RemoveLinkCommand::executeCommand()
{
  smsc_log_debug(_logger, "LM_SGPLinks_RemoveLinkCommand::executeCommand::: linkId=[%s]", _linkId.c_str());
  runtime_cfg::RuntimeConfig::getInstance().notifyRemoveParameterEvent(runtime_cfg::CompositeParameter("config.sgp_links.link", _linkId));
  return "OK";
}

}
