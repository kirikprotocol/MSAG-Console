#include "LM_TranslationTable_RemoveTranslationRule.hpp"
#include "LM_TranslationTable_AddTranslationRule.hpp"

#include "eyeline/ss7na/m3ua_gw/lm_subsystem/RootLayerCommandsInterpreter.hpp"
#include "eyeline/ss7na/m3ua_gw/runtime_cfg/RuntimeConfig.hpp"
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
LM_TranslationTable_RemoveTranslationRule::composeCommandId(const std::string& rule_name)
{
  return "translationTable_RemoveTranslationRule_" + rule_name;
}

std::string
LM_TranslationTable_RemoveTranslationRule::executeCommand()
{
  _trnMgr.addOperation(this, LM_TranslationTable_AddTranslationRule::composeCommandId(_ruleName));
  return "OK";
}

void
LM_TranslationTable_RemoveTranslationRule::updateConfiguration()
{
  std::list<sccp::router::GTTranslationTable*> knownTranslationTables;
  sccp::router::GTTranslationTablesRegistry::getInstance().getKnownTranslationTables(&knownTranslationTables);
  for (std::list<sccp::router::GTTranslationTable*>::iterator iter = knownTranslationTables.begin(), end_iter = knownTranslationTables.end();
      iter != end_iter; ++iter) {
    smsc_log_debug(_logger, "LM_TranslationTable_RemoveTranslationRule::updateConfiguration::: try next translation table, rule_name=%s",
                   _ruleName.c_str());
    if ( (*iter)->removeTranslationEntry(_ruleName) ) {
      smsc_log_debug(_logger, "LM_TranslationTable_RemoveTranslationRule::updateConfiguration::: try rule_name=%s has been removed",
                     _ruleName.c_str());
      break;
    }
  }

  utilx::runtime_cfg::CompositeParameter& translationTable =
      runtime_cfg::RuntimeConfig::getInstance().find<utilx::runtime_cfg::CompositeParameter>("config.translation-table");

  utilx::runtime_cfg::CompositeParameter::Iterator<utilx::runtime_cfg::CompositeParameter>
    entryIterator = translationTable.getIterator<utilx::runtime_cfg::CompositeParameter>("entry");
  while ( entryIterator.hasElement() ) {
    if ( entryIterator.getCurrentElement()->getValue() == _ruleName) {
      entryIterator.removeElement();
      break;
    }
    entryIterator.next();
  }

  RootLayerCommandsInterpreter::configurationWasModified = true;
}

}}}}}
