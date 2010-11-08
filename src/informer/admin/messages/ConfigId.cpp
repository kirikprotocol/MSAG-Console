#include "ConfigId.hpp"

namespace eyeline{
namespace informer{
namespace admin{
namespace messages{

const int8_t ConfigId::ciSmsc;
const int8_t ConfigId::ciRegion;
const int8_t ConfigId::ciUser;
std::map<ConfigId::type,std::string> ConfigId::nameByValue;
std::map<std::string,ConfigId::type> ConfigId::valueByName;
ConfigId::StaticInitializer ConfigId::staticInitializer;

}
}
}
}
