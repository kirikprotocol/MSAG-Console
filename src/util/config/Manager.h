#ifndef CONFIGURATIONMANAGER_H_INCLUDED_C3A81BD4
#define CONFIGURATIONMANAGER_H_INCLUDED_C3A81BD4

#ifndef __GNUC__
#ident "$Id$"
#endif

#include <xercesc/dom/DOM.hpp>

#include "core/buffers/Hash.hpp"
#include "util/config/ConfigException.h"
#include "util/config/Config.h"
#include "core/synchronization/Mutex.hpp"
#include <string>
#include <utility>
#include <vector>

namespace smsc   {
namespace util   {
namespace config {

enum ConfigValueType{
  cvtInt,
  cvtString,
  cvtBool
};

enum ConfigParamWatchType{
  cpwtIndividual,
  cpwtAll
};

typedef std::vector<std::pair<ConfigValueType,std::string> > ParamsVector;

class IConfigChangeNotifier{
public:
  virtual ConfigParamWatchType getWatchedParams(ParamsVector& argParams)=0;
  virtual void paramsChanged()=0;
  virtual void paramChanged(ConfigValueType cvt,const std::string& paramName)=0;
};

class Manager
{
public:
  static void init(const char * configurationFileName)
  {
    if (manager.get() != 0)
    {
      throw ConfigException("Configuration manager already initialized");
    }
    manager.reset(new Manager(configurationFileName));
  }

  static void deinit()
  {
    manager.reset();
  }

  static void reinit();

  static Manager & getInstance()
  {
    return *manager;
  }

  smsc::util::config::Config & getConfig()
  {
    return config;
  }

  int32_t getInt(const char * const paramName)
    throw (ConfigException)
  {
    try {
      return config.getInt(paramName);
    }
    catch (HashInvalidKeyException &e)
    {
      throw ConfigException("Int key '%s' not found",paramName);
    }
  }

  char * getString(const char * const paramName)
    throw (ConfigException)
  {
    try {
      return config.getString(paramName);
    }
    catch (HashInvalidKeyException &e)
    {
      throw ConfigException("String key '%s' not found",paramName);
    }
  }

  bool getBool(const char * const paramName) const
    throw (ConfigException)
  {
    try {
      return config.getBool(paramName);
    }
    catch (HashInvalidKeyException &e)
    {
      throw ConfigException("Bool key '%s' not found",paramName);
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

  void registerConfigChangeWatcher(IConfigChangeNotifier* ccn)
  {
    watchers.push_back(ccn);
  }

protected:
  void findConfigFile();
  Manager(const char* ) throw(ConfigException);
  void readConfig();
  static std::auto_ptr<Manager> manager;
  Config config;
  std::string config_filename;
  static smsc::core::synchronization::Mutex reinitMutex;

  typedef std::vector<IConfigChangeNotifier*> WatchersVector;
  WatchersVector watchers;
};

}
}
}

#endif /* CONFIGURATIONMANAGER_H_INCLUDED_C3A81BD4 */
