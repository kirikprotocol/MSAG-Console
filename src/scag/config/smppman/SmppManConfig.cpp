#include "SmppManConfig.h"

#include "logger/Logger.h"
#include "util/xml/utilFunctions.h"
#include "util/xml/DOMTreeReader.h"

namespace scag {
namespace config {

SmppManConfig::SmppManConfig()
  : logger(smsc::logger::Logger::getInstance("scag.config.SmppManConfig"))
{
}

SmppManConfig::~SmppManConfig()
{
}


}
}
