#include "LM_SGPLinks_Commit.hpp"

#include <sua/sua_layer/runtime_cfg/RuntimeConfig.hpp>

namespace lm_commands {

std::string
LM_SGPLinks_Commit::executeCommand()
{
  smsc_log_debug(_logger, "LM_SGPLinks_Commit::executeCommand::: Enter it");
  runtime_cfg::RuntimeConfig::getInstance().notifyChangeParameterEvent(runtime_cfg::CompositeParameter("config.sgp_links"),
                                                                       runtime_cfg::Parameter("commit", "ok"));
  return "";
}

}
