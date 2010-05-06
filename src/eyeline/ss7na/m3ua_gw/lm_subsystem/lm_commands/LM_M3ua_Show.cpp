#include "LM_M3ua_Show.hpp"

#include "eyeline/ss7na/m3ua_gw/runtime_cfg/RuntimeConfig.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

std::string
LM_M3ua_Show::executeCommand()
{
  std::string resultString;

  utilx::runtime_cfg::CompositeParameter& cfg =
      runtime_cfg::RuntimeConfig::getInstance().find<utilx::runtime_cfg::CompositeParameter>("config");

  utilx::runtime_cfg::Parameter* param =
      cfg.getParameter<utilx::runtime_cfg::Parameter>("local_address");
  if ( param )
    resultString += param->getName() + "=" + param->getValue() + "\n";

  param = cfg.getParameter<utilx::runtime_cfg::Parameter>("local_port");
  if ( param )
    resultString += param->getName() + "=" + param->getValue() + "\n";

  param = cfg.getParameter<utilx::runtime_cfg::Parameter>("lm_address");
  if ( param )
    resultString += param->getName() + "=" + param->getValue() + "\n";

  param = cfg.getParameter<utilx::runtime_cfg::Parameter>("lm_port");
  if ( param )
    resultString += param->getName() + "=" + param->getValue() + "\n";

  param = cfg.getParameter<utilx::runtime_cfg::Parameter>("state_machines_count");
  if ( param )
    resultString += param->getName() + "=" + param->getValue() + "\n";

  param = cfg.getParameter<utilx::runtime_cfg::Parameter>("asp_traffic_mode");
  if ( param )
    resultString += param->getName() + "=" + param->getValue() + "\n";

  param = cfg.getParameter<utilx::runtime_cfg::Parameter>("reassembly_timer");
  if ( param )
    resultString += param->getName() + "=" + param->getValue() + "\n";

  param = cfg.getParameter<utilx::runtime_cfg::Parameter>("reconnect_interval");
  if ( param )
    resultString += param->getName() + "=" + param->getValue() + "\n";

  delete this;
  return resultString + "OK";
}

}}}}}
