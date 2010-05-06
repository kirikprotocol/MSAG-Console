#include "LM_SccpUsers_AddUser.hpp"
#include "eyeline/ss7na/m3ua_gw/runtime_cfg/RuntimeConfig.hpp"
#include "eyeline/ss7na/m3ua_gw/sccp/sap/ApplicationsRegistry.hpp"
#include "eyeline/ss7na/m3ua_gw/lm_subsystem/RootLayerCommandsInterpreter.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

std::string
LM_SccpUsers_AddUser::executeCommand()
{
  _trnMgr.addOperation(this);
  smsc_log_debug(_logger, "LM_SccpUsers_AddUser::executeCommand::: _userId=%s", _userId.c_str());
  return "OK";
}

void
LM_SccpUsers_AddUser::updateConfiguration()
{
  utilx::runtime_cfg::RuntimeConfig& runtimeConfig = runtime_cfg::RuntimeConfig::getInstance();
  utilx::runtime_cfg::CompositeParameter& sccpUsers = runtimeConfig.find<utilx::runtime_cfg::CompositeParameter>("config.sccp_users");

  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::Parameter> iterator =
      sccpUsers.getIterator<utilx::runtime_cfg::Parameter>("userid");
  //iterator points to first element. If there are nothing elements in result then hasElement() returns false
  while(iterator.hasElement()) {
    const utilx::runtime_cfg::Parameter* parameter = iterator.getCurrentElement();
    if ( parameter->getValue() == _userId)
      return;
    iterator.next();
  }

  sccpUsers.addParameter(new utilx::runtime_cfg::Parameter("userid", _userId));
  sccp::sap::ApplicationsRegistry::getInstance().insert(_userId);
  smsc_log_info(_logger, "LM_SccpUsers_AddUser::updateConfiguration::: insert application id='%s' into ApplicationsRegistry, object=0x%p",
                _userId.c_str(), this);
  RootLayerCommandsInterpreter::configurationWasModified = true;
}

std::string
LM_SccpUsers_AddUser::composeCommandId(const std::string& user_id)
{
  return "sccpUsers_addUser" + user_id;
}

}}}}}
