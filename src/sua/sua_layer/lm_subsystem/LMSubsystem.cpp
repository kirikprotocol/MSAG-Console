#include "LMSubsystem.hpp"
#include "TelnetInteraction.hpp"
#include "InputCommandProcessor.hpp"

namespace lm_subsystem {

LMSubsystem::LMSubsystem()
  : sua_layer::ApplicationSubsystem("LMSubsystem", "lm"),
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
LMSubsystem::initialize(runtime_cfg::RuntimeConfig& rconfig)
{
  _host = "niagara";
  _port = 3377;
  InputCommandProcessor::init();
}

}
