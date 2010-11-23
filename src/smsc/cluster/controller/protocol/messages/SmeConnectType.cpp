#include "SmeConnectType.hpp"

namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

const int8_t SmeConnectType::loadBalancer;
const int8_t SmeConnectType::directConnect;
std::map<SmeConnectType::type,std::string> SmeConnectType::nameByValue;
std::map<std::string,SmeConnectType::type> SmeConnectType::valueByName;
SmeConnectType::StaticInitializer SmeConnectType::staticInitializer;

}
}
}
}
}
