#include "ConfigType.hpp"

namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

const int8_t ConfigType::MainConfig;
const int8_t ConfigType::Profiles;
const int8_t ConfigType::Msc;
const int8_t ConfigType::Routes;
const int8_t ConfigType::Sme;
const int8_t ConfigType::ClosedGroups;
const int8_t ConfigType::Aliases;
const int8_t ConfigType::MapLimits;
const int8_t ConfigType::Resources;
const int8_t ConfigType::Reschedule;
const int8_t ConfigType::Snmp;
const int8_t ConfigType::TimeZones;
const int8_t ConfigType::Fraud;
const int8_t ConfigType::Acl;
std::map<ConfigType::type,std::string> ConfigType::nameByValue;
std::map<std::string,ConfigType::type> ConfigType::valueByName;
ConfigType::StaticInitializer ConfigType::staticInitializer;

}
}
}
}
}
