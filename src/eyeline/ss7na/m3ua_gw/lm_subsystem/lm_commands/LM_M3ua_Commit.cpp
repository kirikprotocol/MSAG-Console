#include "LM_M3ua_Commit.hpp"

#include "eyeline/ss7na/m3ua_gw/runtime_cfg/RuntimeConfig.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

std::string
LM_M3ua_Commit::executeCommand()
{
  smsc_log_debug(_logger, "LM_M3ua_Commit::executeCommand::: Enter it");
  _trnMgr.commit();
  return "";
}

}}}}}
