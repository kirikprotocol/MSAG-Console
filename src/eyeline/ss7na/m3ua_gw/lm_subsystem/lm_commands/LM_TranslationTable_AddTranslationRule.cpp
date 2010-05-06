#include "LM_TranslationTable_AddTranslationRule.hpp"
#include "LM_TranslationTable_RemoveTranslationRule.hpp"
#include "eyeline/ss7na/m3ua_gw/lm_subsystem/RootLayerCommandsInterpreter.hpp"
#include "eyeline/ss7na/m3ua_gw/sccp/SccpSubsystem.hpp"
#include "eyeline/ss7na/m3ua_gw/sccp/router/GTTranslationTable.hpp"
#include "eyeline/ss7na/m3ua_gw/sccp/router/GTTranslationTablesRegistry.hpp"
#include "eyeline/ss7na/common/lm_subsystem/Exception.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

std::string
LM_TranslationTable_AddTranslationRule::composeCommandId(const std::string& rule_name)
{
  return "translationTable_AddTranslationRule_" + rule_name;
}

std::string
LM_TranslationTable_AddTranslationRule::executeCommand()
{
  _trnMgr.addOperation(this, LM_TranslationTable_RemoveTranslationRule::composeCommandId(_ruleName));
  return "OK";
}

void
LM_TranslationTable_AddTranslationRule::updateConfiguration()
{
  const std::string& gtAddrFamilyPrefix = sccp::SccpSubsystem::makeAddressFamilyPrefix(_gt);
  sccp::router::GTTranslationTable* translationTable =
      sccp::router::GTTranslationTablesRegistry::getInstance().getGTTranslationTable(gtAddrFamilyPrefix);
  if ( !translationTable )
    throw common::lm_subsystem::InvalidCommandLineException("LM_TranslationTable_AddTranslationRule::updateConfiguration::: translation table not found for address family=[%s]",
                                                            gtAddrFamilyPrefix.c_str());

  if ( _isSsnSet )
    translationTable->addTranslationEntry(_gt, _ssn, _ruleName);
  else
    translationTable->addTranslationEntry(_gt, _ruleName, _toMtp3);

  RootLayerCommandsInterpreter::configurationWasModified = true;
}

}}}}}
