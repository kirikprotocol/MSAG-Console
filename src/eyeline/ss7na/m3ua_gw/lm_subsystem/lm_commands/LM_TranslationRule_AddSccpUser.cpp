#include "LM_TranslationRule_AddSccpUser.hpp"
#include "eyeline/ss7na/m3ua_gw/lm_subsystem/RootLayerCommandsInterpreter.hpp"
#include "eyeline/ss7na/m3ua_gw/sccp/SccpSubsystem.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

std::string
LM_TranslationRule_AddSccpUser::composeCommandId()
{
  return "translationRule_AddSccpUser";
}

void
LM_TranslationRule_AddSccpUser::updateConfiguration()
{
  sccp::SccpSubsystem::getInstance()->addSccpUserPolicy(_routeId, _userIdList, _trafficMode);
  RootLayerCommandsInterpreter::configurationWasModified = true;
}

}}}}}
