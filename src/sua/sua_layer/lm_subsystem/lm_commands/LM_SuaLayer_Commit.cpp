#include "LM_SuaLayer_Commit.hpp"
#include <sua/sua_layer/runtime_cfg/RuntimeConfig.hpp>

namespace lm_commands {

std::string
LM_SuaLayer_Commit::executeCommand()
{
  smsc_log_debug(_logger, "LM_SuaLayer_Commit::executeCommand::: Enter it");
  runtime_cfg::RuntimeConfig::getInstance().notifyChangeParameterEvent(runtime_cfg::CompositeParameter("config"),
                                                                       runtime_cfg::Parameter("commit", "ok"));
  return "";
}

}
