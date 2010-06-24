#include "ConfigType.hpp"

namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{

const int8_t ConfigType::MainConfig=0;
const int8_t ConfigType::Profiles=1;
const int8_t ConfigType::Msc=2;
const int8_t ConfigType::Routes=3;
const int8_t ConfigType::Sme=4;
const int8_t ConfigType::ClosedGroups=5;
const int8_t ConfigType::Aliases=6;
const int8_t ConfigType::MapLimits=7;
const int8_t ConfigType::Resources=8;
const int8_t ConfigType::Reschedule=9;
const int8_t ConfigType::Snmp=10;
const int8_t ConfigType::TimeZones=11;
const int8_t ConfigType::Fraud=12;
const int8_t ConfigType::Acl=13;
std::map<ConfigType::type,std::string> ConfigType::nameByValue;
std::map<std::string,ConfigType::type> ConfigType::valueByName;
ConfigType::StaticInitializer ConfigType::staticInitializer;

}
}
}
}
