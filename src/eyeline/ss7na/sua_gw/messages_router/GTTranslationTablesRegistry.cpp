#include <utility>
#include "core/synchronization/MutexGuard.hpp"
#include "GTTranslationTablesRegistry.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace messages_router {

GTTranslationTablesRegistry::GTTranslationTablesRegistry()
  : _logger(smsc::logger::Logger::getInstance("msg_rout"))
{}

GTTranslationTable*
GTTranslationTablesRegistry::getGTTranslationTable(const std::string& gtAddressFamilyPrefix)
{
  smsc_log_debug(_logger, "GTTranslationTablesRegistry::getGTTranslationTable::: try get GTTranslationTable for gt prefix=[%s]", gtAddressFamilyPrefix.c_str());
  smsc::core::synchronization::MutexGuard guard(_synchonize);

  translation_table_registry_t::iterator iter = _translationTablesRegistry.find(gtAddressFamilyPrefix);
  if ( iter == _translationTablesRegistry.end() )
    return NULL;
  else
    return iter->second;
}
  
void
GTTranslationTablesRegistry::registerGTTranslationTable(const std::string& gtAddressFamilyPrefix,
                                                        GTTranslationTable* translationTable)
{
  smsc_log_debug(_logger, "GTTranslationTablesRegistry::registerGTTranslationTable::: register GTTranslationTable for gt prefix=[%s]", gtAddressFamilyPrefix.c_str());
  smsc::core::synchronization::MutexGuard guard(_synchonize);

  _translationTablesRegistry.insert(std::make_pair(gtAddressFamilyPrefix, translationTable));
}

}}}}
