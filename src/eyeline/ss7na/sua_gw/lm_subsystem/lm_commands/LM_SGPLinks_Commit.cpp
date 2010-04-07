#include "LM_SGPLinks_Commit.hpp"

#include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"
#include "eyeline/ss7na/sua_gw/runtime_cfg/RuntimeConfig.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

std::string
LM_SGPLinks_Commit::executeCommand()
{
  smsc_log_debug(_logger, "LM_SGPLinks_Commit::executeCommand::: Enter it");
  runtime_cfg::RuntimeConfig::getInstance().notifyChangeParameterEvent(utilx::runtime_cfg::CompositeParameter("config.sgp_links"), utilx::runtime_cfg::Parameter("commit", "ok"));
  return "";
}

}}}}}
