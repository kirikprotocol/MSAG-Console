#include "eyeline/utilx/strtol.hpp"
#include "eyeline/utilx/runtime_cfg/Parameter.hpp"
#include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"
#include "LMSubsystem.hpp"
#include "TelnetInteraction.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace lm_subsystem {

LMSubsystem::LMSubsystem()
  : common::ApplicationSubsystem("LMSubsystem", "lm"),
    _userInteractionProcessor(NULL), _port(0)
{}

void
LMSubsystem::start()
{
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
LMSubsystem::initialize(const std::string& host, in_port_t port,
                        InputCommandProcessor& input_cmd_processor)
{
  _host = host; _port = port;
  _userInteractionProcessor = new TelnetInteraction(_host, _port, input_cmd_processor);
}

}}}}
