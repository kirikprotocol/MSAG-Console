#include <sys/types.h>
#include <sys/stat.h>

#include <string>
#include <logger/Logger.h>
#include <util/config/Config.h>
#include <util/config/ConfigView.h>

#include <mcisme/FSStorage.hpp>
#include <mcisme/MCAEventsStorage.hpp>

#include "v2_FormatStorageLoader.hpp"

int main()
{
  smsc::logger::Logger::Init();
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("smsc.mcisme.MCISme");

  try {
    smsc::util::config::Manager::init("./conf/config.xml");
    smsc::util::config::Manager& manager = smsc::util::config::Manager::getInstance();

    smsc::util::config::ConfigView config(manager, "MCISme");

    smsc::util::config::ConfigView mcaEventsStorageConfig(manager, "MCISme.MCAEventStorage");
    smsc::mcisme::MCAEventsStorageRegister::init(mcaEventsStorageConfig);

    std::auto_ptr<smsc::util::config::ConfigView> storageCfgGuard(config.getSubConfig("Storage"));

    std::string location;
    try {
      location = storageCfgGuard->getString("location");
    } catch (...) {
      location = "./";
      smsc_log_warn(logger, "Parameter <MCISme.Storage.location> missed. Default value is './'.");
    }

    smsc::mcisme::FSStorage pStorage;

    if ( pStorage.Init(storageCfgGuard.get(), NULL) ) {
      smsc_log_error(logger, "FSStorage can't be initialized");
      return 1;
    }

    smsc::mcisme::converter::v2_FormatStorageLoader v2_FormatStorageLoader;
    v2_FormatStorageLoader.Init(location);

    v2_FormatStorageLoader.MakeConvertation(&pStorage);

    smsc_log_info(logger, "Convertation successfully completed");
  } catch (std::exception& ex) {
    smsc_log_error(logger, "Convertation failed. Catched unexpected exception [%s]. Exit.", ex.what());
    return 1;
  }

  return 0;
}
