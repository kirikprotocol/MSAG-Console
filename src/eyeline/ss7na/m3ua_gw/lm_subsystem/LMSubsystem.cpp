#include "eyeline/utilx/strtol.hpp"
#include "eyeline/utilx/runtime_cfg/Parameter.hpp"
#include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"
#include "LMSubsystem.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {

void
LMSubsystem::initialize(utilx::runtime_cfg::RuntimeConfig& rconfig)
{
  std::string host;
  try {
    utilx::runtime_cfg::Parameter& lmHostParameter = rconfig.find<utilx::runtime_cfg::Parameter>("config.lm_address");
    host = lmHostParameter.getValue();
  } catch (std::runtime_error& ex) {
    host = "niagara";
  }

  in_port_t port;
  try {
    utilx::runtime_cfg::Parameter& lmPortParameter = rconfig.find<utilx::runtime_cfg::Parameter>("config.lm_port");
    port = static_cast<in_port_t>(lmPortParameter.getIntValue());
    if ( port == 0 && errno )
      throw smsc::util::Exception("LMSubsystem::initialize::: invalid config.lm_port value");
  } catch (std::runtime_error& ex) {
    port = 5555;
  }

  initialize(host, port, _inputCmdProcessor);
}

}}}}
