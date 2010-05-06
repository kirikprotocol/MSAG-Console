#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_ROUTER_GTTRANSLATIONTABLESREGISTRY_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_ROUTER_GTTRANSLATIONTABLESREGISTRY_HPP__

# include <map>
# include <list>
# include <string>
# include "logger/Logger.h"
# include "core/synchronization/Mutex.hpp"
# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/ss7na/m3ua_gw/types.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/router/GTTranslationTable.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace router {

class GTTranslationTablesRegistry : public utilx::Singleton<GTTranslationTablesRegistry> {
public:
  GTTranslationTablesRegistry();

  GTTranslationTable* getGTTranslationTable(const std::string& gt_address_family_prefix);

  void registerGTTranslationTable(const std::string& gt_address_family_prefix,
                                  GTTranslationTable* translation_table);

  void getKnownTranslationTables(std::list<GTTranslationTable*> *known_translation_tables);

private:
  smsc::core::synchronization::Mutex _synchonize;

  typedef std::map<std::string, GTTranslationTable*> translation_table_registry_t;
  translation_table_registry_t _translationTablesRegistry;
  smsc::logger::Logger* _logger;
};

}}}}}

#endif
