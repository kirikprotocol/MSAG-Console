#include <utility>
#include "core/synchronization/MutexGuard.hpp"
#include "GTTranslationTablesRegistry.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace router {

GTTranslationTablesRegistry::GTTranslationTablesRegistry()
  : _logger(smsc::logger::Logger::getInstance("sccp_rout"))
{}

GTTranslationTable*
GTTranslationTablesRegistry::getGTTranslationTable(const std::string& gt_address_family_prefix)
{
  smsc_log_debug(_logger, "GTTranslationTablesRegistry::getGTTranslationTable::: try get GTTranslationTable for gt prefix=[%s]",
                 gt_address_family_prefix.c_str());
  smsc::core::synchronization::MutexGuard guard(_synchonize);

  translation_table_registry_t::iterator iter = _translationTablesRegistry.find(gt_address_family_prefix);
  if ( iter == _translationTablesRegistry.end() )
    return NULL;
  else
    return iter->second;
}
  
void
GTTranslationTablesRegistry::registerGTTranslationTable(const std::string& gt_address_family_prefix,
                                                        GTTranslationTable* translation_table)
{
  smsc_log_debug(_logger, "GTTranslationTablesRegistry::registerGTTranslationTable::: register GTTranslationTable for gt prefix=[%s]",
                 gt_address_family_prefix.c_str());
  smsc::core::synchronization::MutexGuard guard(_synchonize);

  _translationTablesRegistry.insert(std::make_pair(gt_address_family_prefix,
                                                   translation_table));
}

}}}}}
