#ifndef SMSC_LOGGER_ADDITIONAL_LOGGER_CONFIGURATOR
#define SMSC_LOGGER_ADDITIONAL_LOGGER_CONFIGURATOR

#include "core/buffers/Hash.hpp"
#include "util/Properties.h"
#include "util/AutoArrPtr.hpp"

namespace smsc {
namespace logger {

using smsc::util::Properties;

class ConfigReader {
public:
  struct AppenderInfo {
    smsc::util::auto_arr_ptr<char> name;
    smsc::util::auto_arr_ptr<char> type;
    std::auto_ptr<Properties> params;

    AppenderInfo(const char * const name, const char * const type, std::auto_ptr<Properties> params);
  };

  struct CatInfo {
    smsc::util::auto_arr_ptr<char> name;
    smsc::util::auto_arr_ptr<char> level;
    std::auto_ptr<char> appender;

    CatInfo(const char * const name, const char * const initStr);
  };

  ConfigReader(): configReloadInterval(0) {};
  ConfigReader(Properties const & properties);
  ~ConfigReader();

  void init(Properties const & properties);
  void clear();
  void serialize(std::string& str);

  typedef smsc::core::buffers::Hash<AppenderInfo*> AppenderInfos;
  typedef smsc::core::buffers::Hash<CatInfo*> CatInfos;
  AppenderInfos appenders;
  CatInfos cats;

  std::auto_ptr<char> rootLevel;
  std::auto_ptr<char> rootAppender;

  uint32_t configReloadInterval;
private:
  AppenderInfo* createAppender(const char * const name, const Properties & ap);
};

}
}
#endif //SMSC_LOGGER_ADDITIONAL_LOGGER_CONFIGURATOR
