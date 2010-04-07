#include "eyeline/utilx/strtol.hpp"
#include "eyeline/utilx/runtime_cfg/Parameter.hpp"
#include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"
#include "LMSubsystem.hpp"
#include "TelnetInteraction.hpp"
#include "InputCommandProcessor.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {

LMSubsystem::LMSubsystem()
  : common::ApplicationSubsystem("LMSubsystem", "lm"),
    _userInteractionProcessor(NULL)
{}

void
LMSubsystem::start()
{
  _userInteractionProcessor = new TelnetInteraction(_host, _port);
  _userInteractionProcessor->Start();
}

void
LMSubsystem::stop()
{
  _userInteractionProcessor->stop();
}

void
LMSubsystem::waitForCompletion()
{
  _userInteractionProcessor->WaitFor();
}

void
LMSubsystem::initialize(utilx::runtime_cfg::RuntimeConfig& rconfig)
{
  try {
    utilx::runtime_cfg::Parameter& lmHostParameter = rconfig.find<utilx::runtime_cfg::Parameter>("config.lm_ip");
    _host = lmHostParameter.getValue();
  } catch (std::runtime_error& ex) {
    _host = "niagara";
  }

  try {
    utilx::runtime_cfg::Parameter& lmPortParameter = rconfig.find<utilx::runtime_cfg::Parameter>("config.lm_port");
    _port = (in_port_t)utilx::strtol(lmPortParameter.getValue().c_str(), (char **)NULL, 10);
    if ( _port == 0 && errno )
      throw smsc::util::Exception("LMSubsystem::initialize::: invalid config.lm_port value");
  } catch (std::runtime_error& ex) {
    _port = 5555;
  }

  InputCommandProcessor::init();
}

}}}}
