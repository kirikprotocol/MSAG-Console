#ifndef __EYELINE_SUA_SUALAYER_MESSAGESROUTER_GTTRANSLATIONTABLESREGISTRY_HPP__
# define __EYELINE_SUA_SUALAYER_MESSAGESROUTER_GTTRANSLATIONTABLESREGISTRY_HPP__

# include <map>
# include <string>
# include <logger/Logger.h>
# include <core/synchronization/Mutex.hpp>
# include <eyeline/utilx/Singleton.hpp>
# include <eyeline/sua/sua_layer/messages_router/GTTranslationTable.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace messages_router {

class GTTranslationTablesRegistry : public utilx::Singleton<GTTranslationTablesRegistry> {
public:
  GTTranslationTablesRegistry();

  GTTranslationTable* getGTTranslationTable(const std::string& gtAddressFamilyPrefix);

  void registerGTTranslationTable(const std::string& gtAddressFamilyPrefix,
                                  GTTranslationTable* translationTable);
private:
  smsc::core::synchronization::Mutex _synchonize;

  typedef std::map<std::string, GTTranslationTable*> translation_table_registry_t;
  translation_table_registry_t _translationTablesRegistry;
  smsc::logger::Logger* _logger;
};

}}}}

#endif
