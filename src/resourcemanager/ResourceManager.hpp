#ifndef __RESOURCE_MANAGER_HPP__
#define __RESOURCE_MANAGER_HPP__

#include <string>
#include <map>
#include <list>
#include <memory>
#include <resourcemanager/LocaleResources.hpp>
#include <util/Exception.hpp>
#include "core/synchronization/Mutex.hpp"
#include "util/templates/Formatters.h"

using smsc::util::Exception;

namespace smsc{
namespace resourcemanager{

using namespace smsc::util::templates;

class ResourceManager{
public:
  ~ResourceManager() throw ();

  // возвращает строку из сетингов для определленой локали и ключа.
  std::string getSetting(const std::string& locale,const std::string& key) const throw();
  // возвращает строку из сетингов для дефолтной локали и ключа.
  std::string getSetting(const std::string& key ) const throw();
  // возвращает строку из ресурса для определленой локали и ключа.
  std::string getString(const std::string& locale, const std::string& key) const throw();
  // возвращает строку из ресурса для дефолтной локали и ключа.
  std::string getString(const std::string& key) const throw();
  // возвращает инстанс форматтера созданного из строки по
  // данному ключу
  OutputFormatter* getFormatter(const std::string& locale, const std::string& key) const throw();
  OutputFormatter* getFormatter(const std::string& key) const throw();

  bool isValidLocale(const std::string& localeName)const;

  static const ResourceManager* getInstance() throw();
  static void init(const char * const localesString, const char * const defaultLocaleStr) throw();
  static void reload(const char * const localesString, const char * const defaultLocaleStr) throw();

  #ifdef SMSC_DEBUG
  void dump(std::ostream & outStream) const;
  #endif //#ifdef SMSC_DEBUG

private:
  static std::auto_ptr<ResourceManager> instance;
  static smsc::core::synchronization::Mutex mtx;
  static std::string defaultLocale;
  typedef std::map<std::string, LocaleResources*> _LocalesMap;
  _LocalesMap locales;
  std::list<std::string> validLocales;

  static void init(const std::list<std::string> & localeNames, const std::string & defaultLocaleName) throw();
  ResourceManager() throw ();
};

}//resourcemanager
}//smsc

#endif
