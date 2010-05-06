#include "LM_TranslationRule_AddDpc.hpp"
#include "eyeline/ss7na/m3ua_gw/lm_subsystem/RootLayerCommandsInterpreter.hpp"
#include "eyeline/ss7na/m3ua_gw/sccp/SccpSubsystem.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

std::string
LM_TranslationRule_AddDpc::composeCommandId()
{
  return "translationRule_AddDpc";
}

void
LM_TranslationRule_AddDpc::updateConfiguration()
{
  sccp::SccpSubsystem::getInstance()->addMTP3Policy(_routeId,
                                                    _lpc,
                                                    _dpcList,
                                                    _trafficMode);

  RootLayerCommandsInterpreter::configurationWasModified = true;
}

}}}}}
