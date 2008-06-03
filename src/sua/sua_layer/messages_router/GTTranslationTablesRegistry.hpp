#ifndef __SUA_SUALAYER_MESSAGESROUTER_GTTRANSLATIONTABLESREGISTRY_HPP__
# define __SUA_SUALAYER_MESSAGESROUTER_GTTRANSLATIONTABLESREGISTRY_HPP__

# include <map>
# include <string>
# include <logger/Logger.h>
# include <sua/utilx/Singleton.hpp>
# include <sua/sua_layer/messages_router/GTTranslationTable.hpp>
# include <core/synchronization/Mutex.hpp>

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

}

#endif
