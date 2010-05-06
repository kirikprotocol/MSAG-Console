#include "LM_SccpUsers_Show.hpp"
#include "eyeline/utilx/runtime_cfg/CompositeParameter.hpp"
#include "eyeline/ss7na/m3ua_gw/runtime_cfg/RuntimeConfig.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

std::string
LM_SccpUsers_Show::executeCommand()
{
  std::string resultString;

  utilx::runtime_cfg::CompositeParameter& sccpUsers =
    runtime_cfg::RuntimeConfig::getInstance().find<utilx::runtime_cfg::CompositeParameter>("config.sccp_users");

  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::Parameter>
    userIdIterator = sccpUsers.getIterator<utilx::runtime_cfg::Parameter>("userid");
  while ( userIdIterator.hasElement() ) {
    resultString += userIdIterator.getCurrentElement()->getValue() + "\n";
    userIdIterator.next();
  }
  delete this;
  return resultString + "OK";
}

}}}}}
