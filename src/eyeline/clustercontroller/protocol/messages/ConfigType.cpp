#include "ConfigType.hpp"

namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{

const int8_t ConfigType::MainConfig=1;
const int8_t ConfigType::Profiles=2;
const int8_t ConfigType::Msc=3;
const int8_t ConfigType::Routes=4;
const int8_t ConfigType::Sme=5;
const int8_t ConfigType::ClosedGroups=6;
const int8_t ConfigType::Aliases=7;
const int8_t ConfigType::MapLimits=8;
const int8_t ConfigType::Regions=9;
const int8_t ConfigType::Resources=10;
const int8_t ConfigType::Reschedule=11;
const int8_t ConfigType::Snmp=12;
const int8_t ConfigType::TimeZones=13;
const int8_t ConfigType::Fraud=14;
const int8_t ConfigType::Acl=15;
std::map<ConfigType::type,std::string> ConfigType::nameByValue;
std::map<std::string,ConfigType::type> ConfigType::valueByName;
ConfigType::StaticInitializer ConfigType::staticInitializer;

}
}
}
}
