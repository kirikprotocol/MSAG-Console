#include "LM_Applications_Commit.hpp"

#include <sua/sua_layer/runtime_cfg/RuntimeConfig.hpp>

namespace lm_commands {

std::string
LM_Applications_Commit::executeCommand()
{
  smsc_log_debug(_logger, "LM_Applications_Commit::executeCommand::: Enter it");
  runtime_cfg::RuntimeConfig::getInstance().notifyChangeParameterEvent(runtime_cfg::CompositeParameter("config.sua_applications"),
                                                                       runtime_cfg::Parameter("commit", "ok"));
  return "";
}

}
