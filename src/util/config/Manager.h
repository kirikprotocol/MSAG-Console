#ifndef CONFIGURATIONMANAGER_H_INCLUDED_C3A81BD4
#define CONFIGURATIONMANAGER_H_INCLUDED_C3A81BD4

#ifndef __GNUC__
#ident "$Id$"
#endif

#include <iostream>

#include <xercesc/dom/DOM.hpp>

#include "store/StoreConfig.h"
#include "core/buffers/Hash.hpp"
#include "util/cstrings.h"
#include "util/config/ConfigException.h"
#include "util/config/Config.h"

namespace smsc   {
namespace util   {
namespace config {

XERCES_CPP_NAMESPACE_USE
using smsc::store::StoreConfig;
using smsc::core::buffers::Hash;
using smsc::util::cStringCopy;

/**
* Класс, отвечающий за чтение и запись конфигурации системы.
* Чтение производится в конструкторе, запись - методом save()
*
* @author igork
* @see Db
* @see Map
* @see Log
* @see save()
*/
class Manager
{
public:
  static void init(const char * const configurationFileName)
    throw (ConfigException)
  {
    if (manager.get() != 0)
      throw ConfigException("Configuration manager already initialized");
    config_filename.reset(cStringCopy(configurationFileName));
    manager.reset(new Manager());
  }

  static void deinit()
  {
    manager.reset();
    config_filename.reset();
  }

  static void reinit()
    throw (ConfigException)
  {
    std::auto_ptr<char> filename;
    filename = config_filename;
    deinit();
    init(filename.get());
  }

  /**
  * Возвращает проинициализированный Manager
  */
  static Manager & getInstance()
  {
    return *manager;
  }

  smsc::util::config::Config & getConfig()
  {
    return config;
  }

  /**
  *
  * @param paramName имя параметра
  * @return значение параметра типа int
  * @exception HashInvalidKeyException
  *                   if parameter not found
  * @see getString()
  * @see getBool()
  */
  int32_t getInt(const char * const paramName)
    throw (ConfigException)
  {
    try {
      return config.getInt(paramName);
    }
    catch (HashInvalidKeyException &e)
    {
      std::string s("Int key \"");
      s += paramName;
      s += "\" not found";
      throw ConfigException(s.c_str());
    }
  }

  /**
  *
  * @param paramName имя параметра
  * @return значение параметра типа String
  * @exception HashInvalidKeyException
  *                   if parameter not found
  * @see getInt()
  * @see getBool()
  */
  char * getString(const char * const paramName)
    throw (ConfigException)
  {
    try {
      return config.getString(paramName);
    }
    catch (HashInvalidKeyException &e)
    {
      std::string s("String key \"");
      s += paramName;
      s += "\" not found";
      throw ConfigException(s.c_str());
    }
  }

  /**
  *
  * @param paramName имя параметра
  * @return значение параметра типа Bool
  * @exception HashInvalidKeyException
  *                   if parameter not found
  * @see getInt()
  * @see getString()
  */
  bool getBool(const char * const paramName) const
    throw (ConfigException)
  {
    try {
      return config.getBool(paramName);
    }
    catch (HashInvalidKeyException &e)
    {
      std::string s("Bool key \"");
      s += paramName;
      s += "\" not found";
      throw ConfigException(s.c_str());
    }
  }

  void setInt(const char * const paramName, int32_t value)
  {
    config.setInt(paramName, value);
  }

  void setString(const char * const paramName, const char * const value)
  {
    config.setString(paramName, value);
  }

  void setBool(const char * const paramName, bool value)
  {
    config.setBool(paramName, value);
  }

  /**
  * Запись конфигурации
  */
  void save();

  void removeSection(const char * const sectionName)
  {
    config.removeSection(sectionName);
  }

  //checks does the section with given absolute name exist having parameters defined
  bool    findSection(const char * const sectionName)
  {
      return config.findSection(sectionName);
  }
  CStrSet *getRootSectionNames(void)
  {
    return config.getRootSectionNames();
  }
  CStrSet *getChildSectionNames(const char * const sectionName)
  {
    return config.getChildSectionNames(sectionName);
  }
  CStrSet* getChildShortSectionNames(const char * const sectionName)
  {
    return config.getChildShortSectionNames(sectionName);
  }

  CStrSet *getChildIntParamNames(const char * const sectionName)
  {
    return config.getChildIntParamNames(sectionName);
  }
  CStrSet *getChildBoolParamNames(const char * const sectionName)
  {
    return config.getChildBoolParamNames(sectionName);
  }
  CStrSet *getChildStrParamNames(const char * const sectionName)
  {
    return config.getChildStrParamNames(sectionName);
  }

protected:
  void findConfigFile();
  /**
  * Читает конфигурацию.
  *
  * @param config_filename
  *               Имя файла, в котором хранится конфигурация.
  */
  Manager() throw(ConfigException);
  static std::auto_ptr<Manager> manager;
  Config config;

private:
  static std::auto_ptr<char> config_filename;
  // void writeHeader(std::ostream &out);
  // void writeFooter(std::ostream &out);

  DOMDocument * parse(const char * const filename) throw (ConfigException);
};

}
}
}

#endif /* CONFIGURATIONMANAGER_H_INCLUDED_C3A81BD4 */
