#include "LM_TranslationRule_SetLpc.hpp"
#include "eyeline/ss7na/m3ua_gw/lm_subsystem/RootLayerCommandsInterpreter.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

std::string
LM_TranslationRule_SetLpc::composeCommandId(const std::string& lpc)
{
  return "translationRule_SetLpc_" + lpc;
}

void
LM_TranslationRule_SetLpc::updateConfiguration()
{
  RootLayerCommandsInterpreter::configurationWasModified = true;
}

}}}}}
