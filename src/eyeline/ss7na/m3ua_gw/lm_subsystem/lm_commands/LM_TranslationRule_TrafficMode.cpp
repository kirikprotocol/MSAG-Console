#include "LM_TranslationRule_TrafficMode.hpp"
#include "eyeline/ss7na/m3ua_gw/lm_subsystem/RootLayerCommandsInterpreter.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

std::string
LM_TranslationRule_TrafficMode::composeCommandId(const std::string& mode)
{
  return "translationRule_TrafficMode_" + mode;
}

void
LM_TranslationRule_TrafficMode::updateConfiguration()
{
  RootLayerCommandsInterpreter::configurationWasModified = true;
}

}}}}}
